
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

#ifndef REQUEST_H_
#define REQUEST_H_

#include "Zway/event/event.h"
#include "Zway/packet.h"
#include "Zway/thread.h"

namespace Zway {

// ============================================================ //
// Request
// ============================================================ //

const uint32_t DEFAULT_TIMEOUT = 15000;

class Client;

class Request : public std::enable_shared_from_this<Request>
{
public:

    enum Type
    {
        Dispatch = 500,


        CreateAccount = 1000,

        Login = 1010,

        Config = 1020,


        AddContact = 2000,

        CreateAddCode = 2010,

        FindContact = 2020,

        AcceptContact = 2030,

        RejectContact = 2040,

        ContactStatus = 2050,


        GetInbox = 3000,

        GetMessage = 3010
    };

    enum Status
    {
        Inactive,

        Idle,

        Sending,

        WaitingForResponse,

        Completed,

        Timeout,

        Error
    };

    typedef std::shared_ptr<Request> Pointer;

    Request(
            Type type,
            uint32_t timeout = DEFAULT_TIMEOUT,
            uint32_t delay = 0);

    virtual ~Request();

    virtual bool start();

    virtual bool finish();

    virtual bool processSend();

    virtual bool processRecv(PACKET pkt, const UBJ::Value &head);

    virtual bool checkTimeout();

    virtual void invokeCallback(EVENT event);

    bool waitResponse();

    void setStatus(Status status);

    void setClient(Client *client);

    uint32_t id();

    uint32_t type();

    Status status();

    uint32_t timeout();

    bool completed();

protected:

    bool sendPacket(PACKET pkt);

protected:

    uint32_t m_id;

    Type m_type;

    ThreadSafe<Status> m_status;

    Client* m_client;

    uint32_t m_timeout;

    uint32_t m_delay;

    uint32_t m_startTime;

    UBJ::Object m_head;
};

typedef Request::Pointer REQUEST;

typedef std::map<uint32_t, REQUEST> REQUEST_MAP;

// ============================================================ //

}

#endif /* REQUEST_H_ */
