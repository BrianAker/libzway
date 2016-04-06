
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

#include "Zway/request/addcontactrequest.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

ADD_CONTACT_REQUEST AddContactRequest::create(
        STORAGE storage,
        const std::string &addCode,
        const std::string &label,
        const std::string &phone,
        bool createAddCode,
        Callback callback)
{
    return ADD_CONTACT_REQUEST(new AddContactRequest(storage, addCode, label, phone, createAddCode, callback));
}

// ============================================================ //

AddContactRequest::AddContactRequest(
        STORAGE storage,
        const std::string &addCode,
        const std::string &label,
        const std::string &phone,
        bool createAddCode,
        Callback callback)
    : Request(createAddCode ? CreateAddCode : AddContact, DEFAULT_TIMEOUT, 0),
      m_storage(storage),
      m_callback(callback)
{
    m_head["addCode"] = addCode;

    m_head["label"] = label;

    m_head["phone"] = phone;

    // our public key

    Storage::NODE data = storage->getNode(UBJ_OBJ("id" << Zway::Storage::DataNodeId));

    UBJ::Object body;

    if (data->bodyUbj(body)) {

        m_head["publicKey"] = body["k2"];
    }
    else {

        // ...
    }
}

// ============================================================ //

bool AddContactRequest::processRecv(PACKET /*pkt*/, const UBJ::Value &head)
{
    uint32_t status = head["status"].toInt();

    if (status == 1) {

        finish();

        if (head.hasField("requestNewId")) {

            UBJ::Object request = UBJ_OBJ(
                    "requestId"   << head["requestNewId"] <<
                    "requestType" << Request::AddContact <<
                    "src"         << m_client->storage()->accountId() <<
                    "addCode"     << head["addCode"] <<
                    "label"       << head["label"] <<
                    "phone"       << head["phone"]);

            m_storage->addRequest(request);
        }

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

void AddContactRequest::invokeCallback(EVENT event)
{
    if (m_callback) {
        m_callback(
            RequestEvent::cast(event),
            std::dynamic_pointer_cast<AddContactRequest>(shared_from_this()));
    }
}

// ============================================================ //

}
