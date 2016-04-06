
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

#include "Zway/thread.h"

namespace Zway {

// ============================================================ //
// Thread
// ============================================================ //

Thread::Thread()
{
    m_cancel = false;
}

// ============================================================ //

Thread::~Thread()
{

}

// ============================================================ //

bool Thread::run()
{
    {
        MutexLocker locker(m_cancel);

        m_cancel = false;
    }

    m_thread = std::thread(&Thread::onRun, this);

    return true;
}

// ============================================================ //

void Thread::cancelAndJoin()
{
    cancel();

    join();
}

// ============================================================ //

void Thread::cancel()
{
    MutexLocker locker(m_cancel);

    m_cancel = true;
}

// ============================================================ //

void Thread::join()
{
    m_thread.join();
}

// ============================================================ //

bool Thread::testCancel()
{
    MutexLocker locker(m_cancel);

    return m_cancel;
}

// ============================================================ //

}
