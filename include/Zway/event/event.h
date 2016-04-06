
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

#ifndef EVENTS_H_
#define EVENTS_H_

#include "Zway/ubj/value.h"
#include <list>

namespace Zway {

// ============================================================ //

class Event : public std::enable_shared_from_this<Event>
{
public:

    enum EventType {

        RequestTimeout = 1,

        Log,

        LoginSuccess,
        LoginFailure,

        ConnectionSuccess,
        ConnectionFailure,
        ConnectionInterrupted,
        Reconnected,
        Disconnected,

        ContactRequest,
        ContactRequestAccepted,
        ContactRequestRejected,
        ContactStatus,

        MessageIncoming,
        MessageOutgoing,
        MessageSent,
        MessageRecv,
        ResourceSent,
        ResourceRecv,
        ResourceFailure
    };

    typedef std::shared_ptr<Event> Pointer;

    typedef std::function<void (Pointer)> Callback;

    static Pointer create(
        uint32_t id,
        const UBJ::Value &data = UBJ::Object(),
        const UBJ::Value &error = UBJ::Object(),
        Callback callback = nullptr);

    virtual ~Event();

    virtual void dispatch();

    uint32_t id() const;

    UBJ::Value &data();

    UBJ::Value &error();

protected:

    Event(
        uint32_t id,
        const UBJ::Value &data = UBJ::Object(),
        const UBJ::Value &error = UBJ::Object(),
        Callback callback = nullptr);

protected:

    uint32_t m_id;

    UBJ::Value m_data;

    UBJ::Value m_error;

    Callback m_callback;
};

typedef Event::Pointer EVENT;

typedef Event::Callback EVENT_CALLBACK;

// ============================================================ //

#define ERROR_INFO(message) \
    (UBJ_OBJ("message" << message << "file" << __FILE__ << "line" << __LINE__))

#define DUMMY_EVENT(id) \
    (Zway::Event::create(id))

#define ERROR_EVENT(id, message) \
    (Zway::Event::create(id, Zway::UBJ::Object(), ERROR_INFO(message)))

// ============================================================ //

}

#endif /* EVENTS_H_ */
