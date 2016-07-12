
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

#include "Zway/message/message.h"
#include "Zway/packet.h"

#include <fstream>

namespace Zway {

const uint32_t MAX_MESSAGE_PART = PACKET_BASE_SIZE + MAX_PACKET_HEAD + MAX_PACKET_BODY;

// ============================================================ //
// Message
// ============================================================ //

MESSAGE Message::create()
{
    return MESSAGE(new Message());
}

// ============================================================ //

Message::Message()
    : UBJ::Object()
{
    setField("id", 0);

    setField("status", 0);

    setField("time", 0);

    setField("history", 0);

    setField("src", 0);

    setField("dst", 0);
}

// ============================================================ //

void Message::setId(uint32_t id)
{
    setField("id", id);
}

// ============================================================ //

void Message::setStatus(Status status)
{
    setField("status", status);
}

// ============================================================ //

void Message::setTime(uint32_t time)
{
    setField("time", time);
}

// ============================================================ //

void Message::setHistory(uint32_t history)
{
    setField("history", history);
}

// ============================================================ //

void Message::setSrc(uint32_t src)
{
    setField("src", src);
}

// ============================================================ //

void Message::setDst(uint32_t dst)
{
    setField("dst", dst);
}

// ============================================================ //

uint32_t Message::id()
{
    return value("id").toInt();
}

// ============================================================ //

Message::Status Message::status()
{
    return (Message::Status)value("status").toInt();
}

// ============================================================ //

uint32_t Message::time()
{
    return value("time").toInt();
}

// ============================================================ //

uint32_t Message::history()
{
    return value("history").toInt();
}

// ============================================================ //

uint32_t Message::src()
{
    return value("src").toInt();
}

// ============================================================ //

uint32_t Message::dst()
{
    return value("dst").toInt();
}

// ============================================================ //

void Message::addResource(RESOURCE res)
{
    if (res) {

        uint32_t id = res->id();

        m_resMap[id] = res;

        m_resList.push_back(res);
    }
}

// ============================================================ //

void Message::addResources(RESOURCE_LIST resources)
{
    for (auto &res : resources) {

        addResource(res);
    }
}

// ============================================================ //

uint32_t Message::numResources()
{
    return m_resList.size();
}

// ============================================================ //

RESOURCE Message::resourceByIndex(uint32_t index)
{
    if (index < m_resList.size()) {

		return m_resList[index];
	}

    return nullptr;
}

// ============================================================ //

RESOURCE Message::resourceById(uint32_t id)
{
    if (m_resMap.find(id) != m_resMap.end()) {

		return m_resMap[id];
	}

    return nullptr;
}

// ============================================================ //

RESOURCE_LIST Message::resourcesByName(const std::string& name)
{
    RESOURCE_LIST res;

    for (auto &it : m_resList) {

        if (it->name() == name) {

            res.push_back(it);
        }
    }

    return res;
}

// ============================================================ //

RESOURCE_LIST Message::resourcesByType(uint32_t type)
{
    RESOURCE_LIST res;

    for (auto &it : m_resList) {

        if (it->type() == type) {

            res.push_back(it);
        }
    }

    return res;
}

// ============================================================ //

std::string Message::firstText()
{
    // get first text resource

    std::string res;

    RESOURCE_LIST resources = resourcesByType(Resource::TextType);

    for (auto &r : resources) {

        if (r->type() == Resource::TextType) {

            res = std::string((char*)r->data()->data(), r->size());

            break;
        }
    }

    return res;
}

// ============================================================ //

std::mutex &Message::mutex()
{
    return m_mutex;
}

// ============================================================ //

}
