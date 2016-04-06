
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

#ifndef EVENT_DISPATCHER_H_
#define EVENT_DISPATCHER_H_

#include "Zway/event/event.h"
#include "Zway/thread.h"

#include <condition_variable>

namespace Zway {

typedef std::function<void (EVENT)> EVENT_HANDLER;

// ============================================================ //

class EventDispatcher : public Thread
{
public:

    EventDispatcher();

    void addHandler(EVENT_HANDLER handler);

    void post(EVENT event, bool immediately = false);

    void cancel();

    void onRun();

private:

    EVENT getEvent();

    void dispatchEvent(EVENT event);

private:

    ThreadSafe<std::list<EVENT>> m_events;

    ThreadSafe<std::list<EVENT_HANDLER>> m_handlers;

    std::mutex m_waitMutex;

    std::condition_variable m_waitCondition;
};

// ============================================================ //

}

#endif /* EVENT_DISPATCHER_H_ */
