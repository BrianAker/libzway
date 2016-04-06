
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

#include "Zway/request/contactstatusrequest.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

CONTACT_STATUS_REQUEST ContactStatusRequest::create(const UBJ::Value &contacts, STORAGE storage)
{
    return CONTACT_STATUS_REQUEST(new ContactStatusRequest(contacts, storage));
}

// ============================================================ //

ContactStatusRequest::ContactStatusRequest(const UBJ::Value &contacts, STORAGE storage)
    : Request(ContactStatus, DEFAULT_TIMEOUT, 0)
{
    if (!contacts.numValues() && storage) {

        UBJ::Array arr;

        Storage::NODE_LIST contactList = storage->getContacts();

        for (auto &contact : contactList) {

            arr << contact->user1();
        }

        m_head["contacts"] = arr;
    }
    else {

        m_head["contacts"] = contacts;
    }
}

// ============================================================ //

bool ContactStatusRequest::processRecv(PACKET /*pkt*/, const UBJ::Value& /*head*/)
{
    finish();

    return true;
}

// ============================================================ //

}
