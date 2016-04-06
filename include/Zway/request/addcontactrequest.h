
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

#ifndef ADD_CONTACT_REQUEST_H_
#define ADD_CONTACT_REQUEST_H_

#include "Zway/request/request.h"
#include "Zway/request/requestevent.h"
#include "Zway/storage/storage.h"

namespace Zway {

// ============================================================ //

class AddContactRequest : public Request
{
public:

    typedef std::shared_ptr<AddContactRequest> Pointer;

    typedef std::function<void (REQUEST_EVENT, Pointer)> Callback;

    static Pointer create(
            STORAGE storage,
            const std::string &addCode = std::string(),
            const std::string &label = std::string(),
            const std::string &phone = std::string(),
            bool createAddCode = false,
            Callback callback = nullptr);

    bool processRecv(PACKET pkt, const UBJ::Value &head);

    void invokeCallback(EVENT event);

protected:

    AddContactRequest(
            STORAGE storage,
            const std::string &addCode = std::string(),
            const std::string &label = std::string(),
            const std::string &phone = std::string(),
            bool createAddCode = false,
            Callback callback = nullptr);

protected:

    STORAGE m_storage;

    Callback m_callback;
};

typedef AddContactRequest::Pointer ADD_CONTACT_REQUEST;

typedef AddContactRequest::Callback ADD_CONTACT_CALLBACK;

// ============================================================ //

}

#endif /* ADD_CONTACT_REQUEST_H_ */
