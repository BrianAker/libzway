
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

#ifndef PACKET_H_
#define PACKET_H_

#include "ubj/value.h"

namespace Zway {

// ============================================================ //
// Packet
// ============================================================ //

extern const uint32_t PACKET_BASE_SIZE;

extern const uint32_t MAX_PACKET_HEAD;
extern const uint32_t MAX_PACKET_BODY;

class Packet
{
public:

    enum PacketId {

        Heartbeat,

        Request,

        Message
    };

    typedef std::shared_ptr<Packet> Pointer;

    static Pointer create(
            uint32_t id = 0,
            BUFFER head = nullptr,
            BUFFER body = nullptr);

    static Pointer createFromUbj(
            uint32_t id = 0,
            const UBJ::Value &head = UBJ::Object(),
            const UBJ::Value &body = UBJ::Object());

    ~Packet();

    void release();

    void setId(uint32_t id);

    void setHead(BUFFER head);

    void setBody(BUFFER body);

    uint32_t& getId() const;

    uint32_t getHeadSize();

    uint32_t getBodySize();

    BUFFER getHead();

    BUFFER getBody();

    bool getHeadUbj(UBJ::Value &head, bool secure = false);

    bool getBodyUbj(UBJ::Value &body, bool secure = false);

protected:

    Packet();

protected:

    mutable uint32_t m_id;

    uint32_t m_headSize;

    uint32_t m_bodySize;

    BUFFER m_head;

    BUFFER m_body;

};

typedef Packet::Pointer PACKET;

// ============================================================ //

}

#endif /* PACKET_H_ */
