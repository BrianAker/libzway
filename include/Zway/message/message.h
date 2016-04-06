
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

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "Zway/message/resource.h"
#include "Zway/thread.h"

#include <mutex>

namespace Zway {

extern const uint32_t MAX_MESSAGE_PART;

// ============================================================ //

class Message : public UBJ::Object, public EnableLock<Message>
{
public:

    enum Status {

        Idle,

        Incoming,

        Outgoing,

        Sent,

        Recv,

        Failure
    };

    typedef std::shared_ptr<Message> Pointer;

    static Pointer create();

    void setId(uint32_t id);

    void setStatus(Status status);

    void setTime(uint32_t time);

    void setHistory(uint32_t history);

    void setSrc(uint32_t src);

    void setDst(uint32_t dst);

    uint32_t id();

    Status status();

    uint32_t time();

    uint32_t history();

    uint32_t src();

    uint32_t dst();

    void addResource(RESOURCE res);

    void addResources(RESOURCE_LIST resources);

    uint32_t numResources();

    RESOURCE resourceByIndex(uint32_t index);

    RESOURCE resourceById(uint32_t id);

    RESOURCE_LIST resourcesByName(const std::string& name);

    RESOURCE_LIST resourcesByType(uint32_t type);

    std::string firstText();

    std::mutex &mutex();

protected:

    Message();

protected:

    RESOURCE_MAP m_resMap;

    RESOURCE_LIST m_resList;

    std::mutex m_mutex;
};

typedef Message::Pointer MESSAGE;

typedef std::deque<MESSAGE> MESSAGE_LIST;

// ============================================================ //

}

#endif /* MESSAGE_H_ */
