
// ============================================================ //
//
//   d88888D db   d8b   db  .d8b.  db    db
//   YP  d8' 88   I8I   88 d8' `8b `8b  d8'
//      d8'  88   I8I   88 88ooo88  `8bd8'
//     d8'   Y8   I8I   88 88~~~88    88
//    d8' db `8b d8'8b d8' 88   88    88
//   d88888P  `8b8' `8d8'  YP   YP    YP
//
//   open-source, cross-platform, crypto-messenger
//
//   Copyright (C) 2016 Marc Weiler
//
//   This library is free software; you can redistribute it and/or
//   modify it under the terms of the GNU Lesser General Public
//   License as published by the Free Software Foundation; either
//   version 2.1 of the License, or (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   Lesser General Public License for more details.
//
// ============================================================ //

#include "Zway/message/messagereceiver.h"
#include "Zway/message/messageevent.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

MESSAGE_RECEIVER MessageReceiver::create(Client *client, UBJ::Value &head, UBJ::Object &contactPublicKey)
{
    MESSAGE_RECEIVER res = MESSAGE_RECEIVER(new MessageReceiver(client, contactPublicKey));

    if (!res->init(head)) {

        return nullptr;
    }

    return res;
}

// ============================================================ //

MessageReceiver::MessageReceiver(Client *client, UBJ::Object &contactPublicKey)
    : m_client(client),
      m_messagePart(0),
      m_messageParts(0),
      m_partsProcessed(0),
      m_status(0),
      m_completed(false),
      m_publicKey(contactPublicKey),
      m_sha2(Crypto::Digest::DIGEST_SHA256)
{

}

// ============================================================ //

bool MessageReceiver::init(UBJ::Value &head)
{
    if (!head.hasField("messageKey")) {

        return false;
    }

    if (!head.hasField("salt")) {

        return false;
    }

    if (!head.hasField("meta")) {

        return false;
    }

    // decrypt message key with our private key

    m_messageKey = Crypto::RSA::decrypt(m_client->storage()->privateKey(), head["messageKey"].buffer());

    if (!m_messageKey) {

        // TODO error event

        return false;
    }

    m_aes.setKey(m_messageKey);

    m_salt = head["salt"].buffer();

    BUFFER metaData = head["meta"].buffer();

    m_aes.setCtr(m_salt);

    m_aes.decrypt(metaData, metaData, metaData->size());

    if (!UBJ::Value::Reader::read(m_meta, metaData)) {

        // TODO error event

        return false;
    }

    // create index over resource meta data

    UBJ::Array arr = m_meta["resources"];

    for (auto it : arr.values()) {

        m_resourceMetaData[it.second["id"].toInt()] = it.second;
    }

    // create message

    m_messageParts = head["messageParts"].toInt();

    m_msg = Message::create();

    m_msg->setId(head["messageId"].toInt());

    m_msg->setStatus(Message::Incoming);

    m_msg->setHistory(m_client->storage()->latestHistory(head["messageSrc"].toInt()));

    m_msg->setSrc(head["messageSrc"].toInt());

    m_msg->setDst(head["messageDst"].toInt());

    // store message

    m_client->storage()->storeMessage(m_msg);

    // raise event

    m_client->postEvent(MessageEvent::create(Event::MessageIncoming, m_msg));

    return true;
}

// ============================================================ //

bool MessageReceiver::process(PACKET pkt, const UBJ::Value &head)
{
    if (!head.hasField("resourceId")) {

        return false;
    }

    uint32_t resourceId = head["resourceId"].toInt();

    uint32_t resourceSize = head["resourceSize"].toInt();

    uint32_t resourceType = head["resourceType"].toInt();

    uint32_t resourcePart = head["resourcePart"].toInt();

    uint32_t resourceParts = head["resourceParts"].toInt();

    std::string resourceName;

    BUFFER buf = Buffer::create(pkt->getBody());

    if (!buf) {

        // TODO error event

        return false;
    }

    RESOURCE res;

    if (m_skipResource.find(resourceId) == m_skipResource.end()) {

        res = m_msg->resourceById(resourceId);

        if (!res) {

            UBJ::Object resourceMetaData = m_resourceMetaData[resourceId];

            resourceName = resourceMetaData["name"].toString();

            if (resourceType != Resource::TextType) {

                // try to find node with same name

                RESOURCE r = m_client->storage()->getResource(
                        UBJ_OBJ(
                            "name"   << resourceName <<
                            "parent" << m_client->storage()->incomingDir(m_msg->src())));

                if (r) {

                    // check md5

                    BUFFER md5 = resourceMetaData["hash"].buffer();

                    if (r->md5()->equals(md5)) {

                        m_skipResource[resourceId] = true;

                        m_msg->addResource(r);

                        // raise event

                        UBJ::Object data;

                        data["replaced"] =
                                UBJ_OBJ(
                                    "src" << resourceId <<
                                    "dst" << r->id());

                        m_client->postEvent(MessageEvent::create(Event::ResourceRecv, m_msg, r, data));
                    }
                    else {

                        // rename resource
                    }
                }
            }

            if (m_skipResource.find(resourceId) == m_skipResource.end()) {

                // create resource

                res = Resource::create();

                res->setId(resourceId);

                res->setType((Zway::Resource::Type)resourceType);

                res->setName(resourceName);

                m_msg->addResource(res);

                m_resourceParts[res->id()] = 0;

                if (res->type() != Resource::TextType) {

                    uint32_t incomingDir = m_client->storage()->incomingDir(m_msg->src());

                    // create storage node

                    if (!m_client->storage()->storeResource(res, m_msg, resourceSize, incomingDir)) {

                        // TODO error event

                        return false;
                    }

                    // open body blob

                    if (!m_client->storage()->openBodyBlob(resourceId)) {

                        // TODO error event

                        return false;
                    }
                }
                else {

                    BUFFER resourceBuffer = Buffer::create(nullptr, resourceSize);

                    if (!resourceBuffer) {

                        // TODO error event

                        return false;
                    }

                    res->setData(resourceBuffer);
                }
            }

            // increment salt

            incrementSalt();

            m_aes.setCtr(m_salt);
        }
    }

    if (m_skipResource.find(resourceId) == m_skipResource.end()) {

        // update digest

        m_sha2.update(buf->data(), buf->size());

        // decrypt data

        // TODO decrypt directly to storage

        m_aes.decrypt(buf, buf, buf->size());

        // write resource part to storage

        uint32_t offset = resourcePart * MAX_PACKET_BODY;

        if (res->type() != Resource::TextType) {

            if (!m_client->storage()->writeBodyBlob(resourceId, buf->data(), buf->size(), offset)) {

                // TODO error event

                return false;
            }
        }
        else {

            if (!res->data()->write(buf->data(), buf->size(), offset)) {

                // TODO error event

                return false;
            }
        }

        // check whether the resource has completed

        if (++m_resourceParts[res->id()] == resourceParts) {

            // resource completed

            if (res->type() != Resource::TextType) {

                m_client->storage()->closeBodyBlob(resourceId);
            }

            // verify

            BUFFER digest = Buffer::create(nullptr, 32);

            m_sha2.result(digest->data(), digest->size());

            if (!head.hasField("signature")) {

                // TODO: delete resource

                m_client->postEvent(ERROR_EVENT(0, "Missing resource signature!"));

                return false;
            }

            if (!Crypto::RSA::verify(m_publicKey, digest, head["signature"].buffer())) {

                // TODO: delete resource

                m_client->postEvent(ERROR_EVENT(0, "Failed to verify resource signature!"));

                return false;
            }

            if (m_partsProcessed + 1 == m_messageParts) {

                Zway::Message::Lock lock(*m_msg);

                m_msg->setTime(time(nullptr));

                m_msg->setStatus(Message::Recv);
            }

            m_client->postEvent(MessageEvent::create(Event::ResourceRecv, m_msg, res));
        }
    }

    m_partsProcessed++;

    if (m_partsProcessed == m_messageParts) {

        m_completed = true;

        {
            Zway::Message::Lock lock(*m_msg);

            m_msg->setTime(time(nullptr));

            m_msg->setStatus(Message::Recv);

            m_client->storage()->updateMessage(m_msg);
        }

        // raise event

        m_client->postEvent(MessageEvent::create(Event::MessageRecv, m_msg));
    }

	return true;
}

// ============================================================ //

bool MessageReceiver::completed()
{
    return m_completed;
}

// ============================================================ //

void MessageReceiver::incrementSalt()
{
    if (m_salt) {

        uint32_t *p = (uint32_t*)(m_salt->data() + 12);

        (*p)++;
    }
}

// ============================================================ //

}
