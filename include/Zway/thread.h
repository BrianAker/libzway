
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

#ifndef THREAD_H_
#define THREAD_H_

#include <thread>
#include <mutex>

namespace Zway {

typedef std::lock_guard<std::mutex> MutexLocker;

// ============================================================ //
// ThreadSafe
// ============================================================ //

template <typename T>
class ThreadSafe : public std::mutex
{
public:

    ThreadSafe()
        : std::mutex()
    {
    }

    ThreadSafe(T t)
        : std::mutex()
    {
        m_t = t;
    }

    ThreadSafe& operator=(T t)
    {
        m_t = t;

        return *this;
    }

    operator T&() const
    {
        return (T&)m_t;
    }

    operator T&()
    {
        return m_t;
    }

    T& operator*()
    {
        return m_t;
    }

    T* operator->()
    {
        return &m_t;
    }

protected:

    T m_t;
};

// ============================================================ //

class Thread
{
public:

    Thread();

    virtual ~Thread();

    virtual bool run();

    virtual void cancelAndJoin();

    virtual void cancel();

    virtual void join();

    virtual bool testCancel();

    virtual void onRun() = 0;

protected:

    std::thread m_thread;

    ThreadSafe<bool> m_cancel;
};

// ============================================================ //

template <typename T>
class EnableLock
{
public:

    class Lock
    {
    public:

        Lock(T & value) : m_value(value), m_guard(value.mutex()) {}

        T * operator -> () { return &m_value; }

    private:

        T & m_value;

        std::lock_guard<std::mutex> m_guard;
    };

    virtual std::mutex &mutex() = 0;
};

// ============================================================ //

template <typename T>
class EnableRecursiveLock
{
public:

    class Lock
    {
    public:

        Lock(T & value) : m_value(value), m_guard(value.mutex()) {}

        T * operator -> () { return &m_value; }

    private:

        T & m_value;

        std::lock_guard<std::recursive_mutex> m_guard;
    };

    virtual std::recursive_mutex &mutex() = 0;
};

// ============================================================ //

class EnableVisit
{
public:

    template <typename TAction>
    auto visit(TAction action) -> decltype (action())
    {
        std::lock_guard<std::mutex> locker(mutex());
        return action();
    }

private:

    virtual std::mutex &mutex() = 0;
};

// ============================================================ //

}

#endif /* THREAD_H_ */
