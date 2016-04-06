
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

#ifndef MESSAGE_REQUEST_H_
#define MESSAGE_REQUEST_H_

#include "request.h"

namespace Zway {

// ============================================================ //

class MessageRequest : public Request
{
public:

    typedef std::shared_ptr<MessageRequest> Pointer;

    static Pointer create(uint32_t messageId);

    bool processSend();

    bool processRecv(PACKET pkt, const UBJ::Value &head);

protected:

    MessageRequest(uint32_t messageId);

protected:

    uint32_t m_messageId;
};

typedef MessageRequest::Pointer MESSAGE_REQUEST;

// ============================================================ //

}

#endif /* MESSAGE_REQUEST_H_ */
