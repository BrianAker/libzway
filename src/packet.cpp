
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

#include "Zway/packet.h"

namespace Zway {

const uint32_t PACKET_BASE_SIZE = 12;

const uint32_t MAX_PACKET_HEAD = 2048;
const uint32_t MAX_PACKET_BODY = 65536;

// ============================================================ //
// Packet
// ============================================================ //

//! Constructor
/*!
 * \param id            Packet id
 */

Packet::Packet()
    : m_id(0),
      m_headSize(0),
      m_bodySize(0),
      m_head(nullptr),
      m_body(nullptr)
{
}

// ============================================================ //

Packet::~Packet()
{
    release();
}

// ============================================================ //

PACKET Packet::create(uint32_t id, BUFFER head, BUFFER body)
{
    PACKET p(new Packet());

    p->setId(id);

    p->setHead(head);

    p->setBody(body);

    return p;
}

// ============================================================ //

PACKET Packet::createFromUbj(
        uint32_t id,
        const UBJ::Value &head,
        const UBJ::Value &body)
{
    BUFFER headBuf;
    BUFFER bodyBuf;

    if (head.isValid()) {

        headBuf = UBJ::Value::Writer::write(head);
    }

    if (body.isValid()) {

        bodyBuf = UBJ::Value::Writer::write(body);
    }

    return create(id, headBuf, bodyBuf);
}

// ============================================================ //

//! Release packet

void Packet::release()
{
    m_head.reset();
    m_body.reset();

    m_id       = 0;
    m_headSize = 0;
    m_bodySize = 0;
}

// ============================================================ //

//! Set packet id
/*!
 * \param id            Packet id
 */

void Packet::setId(uint32_t id)
{
    m_id = id;
}

// ============================================================ //

//! Set head data
/*!
 *  If data is NULL head will be set to a
 *  zero initialized buffer of size bytes
 */
/*!
 * \param data          Buffer to be copied
 * \param size          The size of the buffer in bytes
 */

void Packet::setHead(BUFFER head)
{
    if (head) {

        m_head = head;

        m_headSize = head->size();
    }
}

// ============================================================ //

//! Set body data
/*!
 *  If data is NULL body will be set to a
 *  zero initialized buffer of size bytes
 */
/*!
 * \param data          Buffer to be copied
 * \param size          The size of the buffer in bytes
 */

void Packet::setBody(BUFFER body)
{
    if (body) {

        m_body = body;

        m_bodySize = body->size();
    }
}

// ============================================================ //

uint32_t& Packet::getId() const
{
    return m_id;
}

// ============================================================ //

uint32_t Packet::getHeadSize()
{
    return m_headSize;
}

// ============================================================ //

uint32_t Packet::getBodySize()
{
    return m_bodySize;
}

// ============================================================ //

BUFFER Packet::getHead()
{
    return m_head;
}

// ============================================================ //

BUFFER Packet::getBody()
{
    return m_body;
}

// ============================================================ //

bool Packet::getHeadUbj(UBJ::Value &head, bool /*secure*/)
{
    if (m_head && m_head->data()) {

        return UBJ::Value::Reader::read(head, m_head);
    }

    return false;
}

// ============================================================ //

bool Packet::getBodyUbj(UBJ::Value &body, bool /*secure*/)
{
    if (m_body && m_body->data()) {

        return UBJ::Value::Reader::read(body, m_body);
    }

    return false;
}

// ============================================================ //

}
