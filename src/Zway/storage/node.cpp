
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

#include "Zway/storage/storage.h"
#include <sqlite3.h>
#include <fstream>
#include <sstream>

namespace Zway {

// ============================================================ //


Storage::NODE Storage::Node::create(uint32_t type, uint32_t parent, const std::string &name)
{
    return NODE(new Node(type, parent, name));
}

Storage::Node::Node(uint32_t type, uint32_t parent, const std::string& name)
    : m_id(Crypto::mkId()),
      m_type(type),
      m_parent(parent),
      m_name(name),
      m_user1(0),
      m_user2(0)
{

}

Storage::Node::~Node()
{

}

Storage::NODE Storage::Node::copy()
{
    return NODE(new Node(*this));
}

uint32_t Storage::Node::id() const
{
    return m_id;
}

uint32_t Storage::Node::type() const
{
    return m_type;
}

uint32_t Storage::Node::parent() const
{
    return m_parent;
}

std::string Storage::Node::name() const
{
    return m_name;
}

uint32_t Storage::Node::user1() const
{
    return m_user1;
}

uint32_t Storage::Node::user2() const
{
    return m_user2;
}

std::string Storage::Node::user3() const
{
    return m_user3;
}

std::string Storage::Node::user4() const
{
    return m_user4;
}

BUFFER Storage::Node::head()
{
    return m_head;
}

BUFFER Storage::Node::body()
{
    return m_body;
}

bool Storage::Node::headUbj(UBJ::Value &head, bool /*secure*/)
{
    if (m_head && m_head->data()) {

        UBJ::Value::Reader::read(head, m_head);

        return true;
    }

    return false;
}

bool Storage::Node::bodyUbj(UBJ::Value &body, bool /*secure*/)
{
    if (m_body && m_body->data()) {

        UBJ::Value::Reader::read(body, m_body);

        return true;
    }

    return false;
}

void Storage::Node::setId(uint32_t id)
{
    m_id = id;
}

void Storage::Node::setType(uint32_t type)
{
    m_type = type;
}

void Storage::Node::setParent(uint32_t parent)
{
    m_parent = parent;
}

void Storage::Node::setName(const std::string& name)
{
    m_name = name;
}


void Storage::Node::setUser1(uint32_t val)
{
    m_user1 = val;
}

void Storage::Node::setUser2(uint32_t val)
{
    m_user2 = val;
}

void Storage::Node::setUser3(const std::string& val)
{
    m_user3 = val;
}

void Storage::Node::setUser4(const std::string& val)
{
    m_user4 = val;
}

bool Storage::Node::setHead(const uint8_t* data, uint32_t size)
{
    m_head = Buffer::create(data, size);

    return true;
}

bool Storage::Node::setHead(BUFFER buffer)
{
    m_head = buffer;

    return true;
}

bool Storage::Node::setBody(const uint8_t* data, uint32_t size)
{
    m_body = Buffer::create(data, size);

    return true;
}

bool Storage::Node::setBody(BUFFER buffer)
{
    m_body = buffer;

    return true;
}

bool Storage::Node::setHeadUbj(const UBJ::Value &obj)
{
    if (obj.isValid()) {

        Zway::BUFFER buf = UBJ::Value::Writer::write(obj);

        setHead(buf);
    }

    return true;
}

bool Storage::Node::setBodyUbj(const UBJ::Value &obj)
{
    if (obj.isValid()) {

        Zway::BUFFER buf = UBJ::Value::Writer::write(obj);

        setBody(buf);
    }

    return true;
}

// ============================================================ //

}
