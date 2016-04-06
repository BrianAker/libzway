
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

#ifndef MESSAGE_EVENT_H_
#define MESSAGE_EVENT_H_

#include "Zway/event/event.h"
#include "Zway/message/message.h"

namespace Zway {

// ============================================================ //

class MessageEvent : public Event
{
public:

    typedef std::shared_ptr<MessageEvent> Pointer;

    static Pointer cast(EVENT event);

    static Pointer create(uint32_t id, MESSAGE msg, RESOURCE res = nullptr, const UBJ::Object &data = UBJ::Object());

    MESSAGE getMessage();

    RESOURCE getResource();

protected:

    MessageEvent(uint32_t id, MESSAGE msg, RESOURCE res = nullptr, const UBJ::Object &data = UBJ::Object());

protected:

    MESSAGE m_msg;

    RESOURCE m_res;
};

typedef MessageEvent::Pointer MESSAGE_EVENT;

// ============================================================ //

}

#endif /* MESSAGE_EVENT_H_ */
