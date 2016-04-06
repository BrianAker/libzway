
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

#include "Zway/request/findcontactrequest.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

FIND_CONTACT_REQUEST FindContactRequest::create(const UBJ::Value &query, EVENT_CALLBACK callback)
{
    return FIND_CONTACT_REQUEST(new FindContactRequest(query, callback));
}

// ============================================================ //

FindContactRequest::FindContactRequest(const UBJ::Value &query, EVENT_CALLBACK callback)
    : Request(FindContact, DEFAULT_TIMEOUT, 0),
      m_callback(callback)
{
    m_head["query"] = query;
}

// ============================================================ //

bool FindContactRequest::processRecv(PACKET /*pkt*/, const UBJ::Value &head)
{
    uint32_t status = head["status"].toInt();

    finish();

    m_client->postEvent(RequestEvent::create(
            0,
            shared_from_this(),
            status == 0 ? UBJ::Object() : head,
            status == 1 ? UBJ::Object() : ERROR_INFO(head["message"]),
            [this] (EVENT event) {
                invokeCallback(event);
            }));

    return true;
}

// ============================================================ //

void FindContactRequest::invokeCallback(EVENT event)
{
    if (m_callback) {
        m_callback(RequestEvent::cast(event));
    }
}

// ============================================================ //

}
