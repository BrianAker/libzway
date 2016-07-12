
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

#include "Zway/request/messagerequest.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

MESSAGE_REQUEST MessageRequest::create(uint32_t messageId)
{
    return MESSAGE_REQUEST(new MessageRequest(messageId));
}

// ============================================================ //

MessageRequest::MessageRequest(
        uint32_t messageId)
    : Request(GetMessage),
      m_messageId(messageId)
{

}

// ============================================================ //

bool MessageRequest::processSend()
{
    /*
    if (getStatus() == 0) {

        start();

        UBJ::Object head;

        head["messageId"] = m_messageId;

        m_client->sendPacket(Packet::createFromBson(PID_GET_MESSAGE, &head));

        finish();
    }
    */

    return true;
}

// ============================================================ //

bool MessageRequest::processRecv(PACKET /*pkt*/, const UBJ::Value &/*head*/)
{
    /*
    if (pkt->getId() == PID_GET_MESSAGE) {

        finish();

        // ...
    }
    */

    return true;
}

// ============================================================ //

}
