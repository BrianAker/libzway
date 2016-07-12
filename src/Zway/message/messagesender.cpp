
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

#include "Zway/message/messagesender.h"
#include "Zway/message/messageevent.h"
#include "Zway/client.h"

#include <vector>

namespace Zway {

// ============================================================ //

MESSAGE_SENDER MessageSender::create(Client *client, MESSAGE msg)
{
    MESSAGE_SENDER res = MESSAGE_SENDER(new MessageSender(client, msg));

    if (!res->init()) {

        return nullptr;
    }

    return res;
}

// ============================================================ //

MessageSender::MessageSender(Client* client, MESSAGE message)
    : m_client(client),
      m_messagePart(0),
      m_messageParts(0),
      m_resourcePart(0),
      m_resourceIndex(0),
      m_status(0),
      m_messageSize(0),
      m_completed(false),
      m_msg(message),
      m_sha2(Crypto::Digest::DIGEST_SHA256)
{

}

// ============================================================ //

bool MessageSender::init()
{
    // create message id

    if (!m_msg->id()) {

        m_msg->setId(Crypto::mkId());
    }

    // process resources

    for (uint32_t i=0; i<m_msg->numResources(); ++i) {

        RESOURCE res = m_msg->resourceByIndex(i);

        // check whether we are going to write the resource
        // to encrypted storage

        // try to find node with same name

        Zway::Storage::NODE node = m_client->storage()->getNode(
                UBJ_OBJ(
                    "type"   << Storage::Node::ResourceType <<
                    "name"   << res->name() <<
                    "parent" << m_client->storage()->outgoingDir(m_msg->dst())),
                UBJ::Object(),
                UBJ_ARR("user3"));

        if (node) {

            // has this node the same hash

            if (node->user3() == Zway::Crypto::hexStr(res->md5())) {

                // same content, no write

                m_noStoreResource[res->id()] = true;
            }
            else {

                // rename node

                // ...
            }
        }

        int32_t parts = 0;

        if (res->size() > 0) {

            m_messageSize += res->size();

            // determine resource parts

            if (res->size() >= MAX_PACKET_BODY) {

                parts += res->size() / MAX_PACKET_BODY;

                if (res->size() % MAX_PACKET_BODY > 0) {

                    parts++;
                }
            }
            else {

                parts++;
            }

            // create resource id

            if (!res->id()) {

                res->setId(Crypto::mkId());
            }

            m_resourceParts[res->id()] = parts;

            m_messageParts += parts;
        }
    }

    if (m_messageParts) {

        // start with the first resource

        m_res = m_msg->resourceByIndex(0);
    }
    else {

        // nothing to do

        return false;
    }

    // create random message key and apply it to the cipher

    m_messageKeyPlain = Buffer::create(nullptr, 32);

    if (!m_messageKeyPlain) {

        // TODO error event

        return false;
    }

    if (!Crypto::Random::random(m_messageKeyPlain->data(), m_messageKeyPlain->size(), Crypto::Random::Strong)) {

        // TODO error event

        return false;
    }

    m_aes.setKey(m_messageKeyPlain);

    m_aes.setCtr(Buffer::create(nullptr, 16));

    //

    BUFFER key = Crypto::RSA::encrypt(m_client->storage()->publicKey(), m_messageKeyPlain);

    if (!key) {

        // TODO error event

        return false;
    }

    m_messageKeysEnc[m_msg->src()] = key;

    //

    std::vector<uint32_t> dsts = {m_msg->dst()};

    for (auto &dst : dsts) {

        // get contact public key in order to encrypt the message key

        UBJ::Object contact;

        if (m_client->storage()->getContact(dst, contact)) {

            if (!contact.hasField("publicKey")) {

                // TODO error event

                return false;
            }

            // encrypt the key using the contact public key

            BUFFER key = Crypto::RSA::encrypt(contact["publicKey"], m_messageKeyPlain);

            if (!key) {

                // TODO error event

                return false;
            }

            m_messageKeysEnc[dst] = key;
        }
    }

    m_msg->setSrc(m_client->storage()->accountId());


    m_msg->setStatus(Message::Outgoing);


    m_client->storage()->storeMessage(m_msg);


    return true;
}

// ============================================================ //

bool MessageSender::process()
{
	if (m_messageParts == 0) {

		return false;
	}

    // process message part

    // offset into resource data buffer

    uint32_t bodySize = MAX_PACKET_BODY;

    uint32_t offset = m_resourcePart * MAX_PACKET_BODY;

    // compute remaining number of bytes to send for the current resource

    uint32_t remainingBytes = m_res->size() - offset;

    if (remainingBytes < MAX_PACKET_BODY) {

    	bodySize = remainingBytes;
    }

    // do some special stuff for first resource packet

    if (m_resourcePart == 0) {

        // open storage blob

        if (m_res->type() != Resource::TextType) {

            if (m_noStoreResource.find(m_res->id()) == m_noStoreResource.end()) {

                uint32_t outgoingDir = m_client->storage()->outgoingDir(m_msg->dst());

                if (!m_client->storage()->storeResource(m_res, m_msg, m_res->size(), outgoingDir)) {

                    // TODO error event

                    return false;
                }

                if (!m_client->storage()->openBodyBlob(m_res->id())) {

                    // TODO error event

                    return false;
                }
            }
        }

        // encrypt resource name

        std::string name = m_res->name();

        m_aes.setCtr(Buffer::create(nullptr, 16));
        m_aes.encrypt(&name[0], &name[0], name.size());

        m_resourceNameEnc[m_res->id()] = Buffer::create((uint8_t*)&name[0], name.size());

        // encrypt resource md5

        BUFFER md5 = m_res->md5()->copy();

        m_aes.setCtr(Buffer::create(nullptr, 16));
        m_aes.encrypt(md5, md5, md5->size());

        m_resourceHashEnc[m_res->id()] = md5;

        // reset ctr

        m_aes.setCtr(Buffer::create(nullptr, 16));
    }

    // pointer into resource buffer

    uint8_t* ptr = m_res->data()->data() + offset;

    // write to storage

    if (m_res->type() != Resource::TextType) {

        if (m_noStoreResource.find(m_res->id()) == m_noStoreResource.end()) {

            m_client->storage()->writeBodyBlob(m_res->id(), ptr, bodySize, offset);
        }
    }

    BUFFER buf = Buffer::create(ptr, bodySize);

    // encrypt data

    m_aes.encrypt(buf, buf, buf->size());

    // update sha2 with encrypted data

    m_sha2.update(buf->data(), buf->size());

    // prepare packet head

    UBJ::Object head;

    // add message keys if first part

    if (m_messagePart == 0) {

        UBJ::Array keys;

        for (auto &it : m_messageKeysEnc) {

            keys << UBJ_OBJ("dst" << it.first << "key" << it.second);
        }

        head["keys"] = keys;
    }

    // message info

    head["messageId"]    = m_msg->id();
    head["messageTime"]  = m_msg->time();
    head["messageSrc"]   = m_msg->src();
    head["messageDst"]   = m_msg->dst();
    head["messagePart"]  = m_messagePart;
    head["messageParts"] = m_messageParts;

    // resource info

    head["resourceId"]    = m_res->id();
    head["resourceType"]  = m_res->type();
    head["resourceSize"]  = m_res->size();
    head["resourceName"]  = m_resourceNameEnc[m_res->id()];
    head["resourceHash"]  = m_resourceHashEnc[m_res->id()];
    head["resourcePart"]  = m_resourcePart;
    head["resourceParts"] = m_resourceParts[m_res->id()];

    // sign resource if last part

    if (m_resourcePart == m_resourceParts[m_res->id()] - 1) {

        // get digest for encrypted resource data

        BUFFER digest = Buffer::create(nullptr, Crypto::Digest::DIGEST_SHA256_SIZE);

        if (!digest) {

            // TODO error event

            return false;
        }

        m_sha2.result(digest->data(), digest->size());

        // create signature

        BUFFER signature = Crypto::RSA::sign(m_client->storage()->privateKey(), digest);

        if (!signature) {

            // TODO error event

            return false;
        }

        // append signature

        head["signature"] = signature;
    }

    // send message part

    PACKET pkt = Packet::create(
            Packet::Message,
            UBJ::Value::Writer::write(head),
            buf);

    if (m_client->sendPacket(pkt) <= 0) {

        m_msg->setStatus(Message::Failure);

        m_client->postEvent(MessageEvent::create(Event::ResourceFailure, m_msg, m_res));

        return false;
    }

	m_messagePart++;

	m_resourcePart++;

    if (m_resourcePart == m_resourceParts[m_res->id()]) {

    	// resource completed

        if (m_res->type() != Resource::TextType) {

            if (m_noStoreResource.find(m_res->id()) == m_noStoreResource.end()) {

                m_client->storage()->closeBodyBlob(m_res->id());
            }
        }

        // message completed

        if (m_messagePart == m_messageParts) {

            Zway::Message::Lock lock(*m_msg);

            m_msg->setTime(time(nullptr));

            m_msg->setStatus(Message::Sent);

            m_client->storage()->updateMessage(m_msg);
        }

        // raise event

        m_client->postEvent(MessageEvent::create(Event::ResourceSent, m_msg, m_res));

    	// next resource

    	m_resourcePart = 0;

    	m_resourceIndex++;

        m_res = m_msg->resourceByIndex(m_resourceIndex);

        if (m_res) {

            // create resource id

            if (!m_res->id()) {

                m_res->setId(Crypto::mkId());
            }
        }

        // reset cipher

        m_aes.setCtr(Buffer::create(nullptr, 16));
    }

    if (m_messagePart == m_messageParts) {

        m_completed = true;

        // raise event

        m_client->postEvent(MessageEvent::create(Event::MessageSent, m_msg));
    }

    return true;
}

// ============================================================ //

bool MessageSender::completed()
{
    return m_completed;
}

// ============================================================ //

MESSAGE MessageSender::getMessage()
{
    return m_msg;
}

// ============================================================ //

}
