
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

#include "Zway/message/messageevent.h"

namespace Zway {

// ============================================================ //

MESSAGE_EVENT MessageEvent::cast(EVENT event)
{
    return std::dynamic_pointer_cast<MessageEvent>(event);
}

// ============================================================ //

MESSAGE_EVENT MessageEvent::create(uint32_t id, MESSAGE msg, RESOURCE res, const UBJ::Object &data)
{
    return MESSAGE_EVENT(new MessageEvent(id, msg, res, data));
}

// ============================================================ //

MessageEvent::MessageEvent(uint32_t id, MESSAGE msg, RESOURCE res, const UBJ::Object &data)
    : Event(id),
      m_msg(msg),
      m_res(res)
{
    m_data = data;
}

// ============================================================ //

MESSAGE MessageEvent::getMessage()
{
    return m_msg;
}

// ============================================================ //

RESOURCE MessageEvent::getResource()
{
    return m_res;
}

// ============================================================ //

}
