
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

#include "Zway/request/acceptcontactrequest.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

ACCEPT_CONTACT_REQUEST AcceptContactRequest::create(
        uint32_t requestId,
        STORAGE storage,
        Callback callback)
{
    return ACCEPT_CONTACT_REQUEST(new AcceptContactRequest(requestId, storage, callback));
}

// ============================================================ //

AcceptContactRequest::AcceptContactRequest(
        uint32_t requestId,
        STORAGE storage,
        Callback callback)
    : Request(AcceptContact, DEFAULT_TIMEOUT, 0),
      m_callback(callback)
{
    m_head["requestOrigId"] = requestId;

    Storage::NODE data = storage->getNode(UBJ_OBJ("id" << Zway::Storage::DataNodeId));

    UBJ::Object body;

    if (data->bodyUbj(body)) {

        m_head["publicKey"] = body["k2"];
    }
}

// ============================================================ //

bool AcceptContactRequest::processRecv(PACKET /*pkt*/, const UBJ::Value &head)
{
    uint32_t status = head["status"].toInt();

    if (status == 1) {

        finish();

        m_client->storage()->deleteRequest(m_head["requestOrigId"].toInt());

        m_client->storage()->addContact(
                UBJ_OBJ(
                    "contactId" << head["contactId"] <<
                    "label"     << head["label"] <<
                    "phone"     << head["phone"] <<
                    "publicKey" << head["publicKey"]));

        m_client->setContactStatus(head["contactId"].toInt(), head["contactStatus"].toInt());

        m_client->setConfig();

        m_client->postEvent(RequestEvent::create(
                0,
                shared_from_this(),
                head,
                UBJ::Object(),
                [this] (EVENT event) {
                    invokeCallback(event);
                }));
    }
    else
    if (status == 0) {

        finish();

        m_client->storage()->deleteRequest(m_head["requestOrigId"].toInt());

        m_client->postEvent(RequestEvent::create(
                0,
                shared_from_this(),
                UBJ::Object(),
                ERROR_INFO(head["message"]),
                [this] (EVENT event) {
                    invokeCallback(event);
                }));
    }

    return true;
}

// ============================================================ //

void AcceptContactRequest::invokeCallback(EVENT event)
{
    if (m_callback) {
        m_callback(
            RequestEvent::cast(event),
            std::dynamic_pointer_cast<AcceptContactRequest>(shared_from_this()));
    }
}

// ============================================================ //

}

