
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

#include "Zway/request/loginrequest.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

LOGIN_REQUEST LoginRequest::create(STORAGE storage, Callback callback)
{
    return LOGIN_REQUEST(new LoginRequest(storage, callback));
}

// ============================================================ //

LoginRequest::LoginRequest(STORAGE storage, Callback callback)
    : Request(Login, DEFAULT_TIMEOUT, 0),
      m_storage(storage),
      m_callback(callback)
{
    m_head["accountId"] = storage->accountId();

    m_head["accountPw"] = storage->accountPw();

    UBJ::Object config;

    storage->getConfig(config);

    m_head["config"] = config;

    UBJ::Object contacts;

    Storage::NODE_LIST nodes = storage->getContacts();

    for (auto &node : nodes) {

        contacts[node->user1()] = UBJ_OBJ("notifyStatus" << 1);
    }

    m_head["config"]["contacts"] = contacts;
}

// ============================================================ //

bool LoginRequest::processRecv(PACKET /*pkt*/, const UBJ::Value &head)
{
    uint32_t status = head["status"].toInt();

    if (status == 1) {

        finish();

        m_client->setStatus(Client::LoggedIn);

        // raise event

        m_client->postEvent(RequestEvent::create(
                Event::LoginSuccess,
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
                Event::LoginFailure,
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

void LoginRequest::invokeCallback(EVENT event)
{
    if (m_callback) {
        m_callback(
            RequestEvent::cast(event),
            std::dynamic_pointer_cast<LoginRequest>(shared_from_this()));
    }
}

// ============================================================ //

}
