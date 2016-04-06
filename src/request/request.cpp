
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

#include "Zway/request/request.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //
// Request
// ============================================================ //

Request::Request(
        Type type,
        uint32_t timeout,
        uint32_t delay)
    : m_id(Crypto::mkId()),
      m_type(type),
      m_status(Inactive),
      m_client(nullptr),
      m_timeout(timeout),
      m_delay(delay),
      m_startTime(0)
{

}

// ============================================================ //

Request::~Request()
{

}

// ============================================================ //

bool Request::start()
{
    m_startTime = Client::tickCount();

    setStatus(Idle);

    return true;
}

// ============================================================ //

bool Request::finish()
{
    setStatus(Completed);

    return true;
}

// ============================================================ //

bool Request::processSend()
{
    if (status() == Idle) {

        m_head["requestId"] = m_id;

        m_head["requestType"] = m_type;

        if (!sendPacket(Packet::createFromUbj(Packet::Request, m_head))) {

            return false;
        }

        return waitResponse();
    }

    return false;
}

// ============================================================ //

bool Request::processRecv(PACKET /*pkt*/, const UBJ::Value &/*head*/)
{
    return false;
}

// ============================================================ //

bool Request::checkTimeout()
{
    if (m_timeout > 0 &&
        m_startTime > 0 &&
        Client::tickCount() >= m_startTime + m_timeout) {

        setStatus(Timeout);

        return true;
    }

    return false;
}


// ============================================================ //

void Request::invokeCallback(EVENT event)
{

}

// ============================================================ //

bool Request::waitResponse()
{
    if (status() == Idle) {

        setStatus(WaitingForResponse);

        return true;
    }

    return false;
}

// ============================================================ //

void Request::setStatus(Request::Status status)
{
    MutexLocker locker(m_status);

    m_status = status;
}

// ============================================================ //

void Request::setClient(Client *client)
{
    m_client = client;
}

// ============================================================ //

uint32_t Request::id()
{
    return m_id;
}

// ============================================================ //

uint32_t Request::type()
{
    return m_type;
}

// ============================================================ //

Request::Status Request::status()
{
    MutexLocker locker(m_status);

    return m_status;
}

// ============================================================ //

uint32_t Request::timeout()
{
    return m_timeout;
}

// ============================================================ //

bool Request::completed()
{
    return status() == Completed;
}

// ============================================================ //

bool Request::sendPacket(PACKET pkt)
{
    setStatus(Sending);

    if (pkt && m_client && m_client->sendPacket(pkt) > 0) {

        setStatus(Idle);

        return true;
    }

    setStatus(Error);

    return false;
}

// ============================================================ //

}
