
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

#include "Zway/event/eventdispatcher.h"

namespace Zway {

// ============================================================ //

EventDispatcher::EventDispatcher()
    : Thread()
{

}

// ============================================================ //

void EventDispatcher::addHandler(EVENT_HANDLER handler)
{
    MutexLocker locker(m_handlers);

    m_handlers->push_back(handler);
}

// ============================================================ //

void EventDispatcher::post(EVENT event, bool immediately)
{
    if (event) {

        if (immediately) {

            dispatchEvent(event);
        }
        else {

            {
                MutexLocker locker(m_events);

                m_events->push_back(event);
            }

            m_waitCondition.notify_one();
        }
    }
}

// ============================================================ //

void EventDispatcher::cancel()
{
    {
        MutexLocker locker(m_events);

        m_events->clear();
    }

    Thread::cancel();

    m_waitCondition.notify_one();
}

// ============================================================ //

void EventDispatcher::onRun()
{
    for (;;) {

        if (testCancel()) {

            break;
        }

        EVENT event;

        // grab event if any

        if (!(event = getEvent())) {

            // wait for event

            std::unique_lock<std::mutex> locker(m_waitMutex);

            m_waitCondition.wait(locker);

            // grab event if any

            event = getEvent();
        }

        if (event) {

            // dispatch event

            dispatchEvent(event);
        }
    }
}

// ============================================================ //

EVENT EventDispatcher::getEvent()
{
    EVENT event;

    MutexLocker locker(m_events);

    if (!m_events->empty()) {

        event = m_events->front();

        m_events->pop_front();
    }

    return event;
}

// ============================================================ //

void EventDispatcher::dispatchEvent(EVENT event)
{
    event->dispatch();

    // invoke event handlers

    {
        MutexLocker locker(m_handlers);

        for (auto &h : *m_handlers) {

            h(event);
        }
    }
}

// ============================================================ //

}
