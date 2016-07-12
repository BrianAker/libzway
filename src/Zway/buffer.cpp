
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

#include "Zway/buffer.h"

#include <cstring>

namespace Zway {

// ============================================================ //

BUFFER Buffer::create(BUFFER buffer)
{
    if (buffer) {

        return create(buffer->data(), buffer->size());
    }

    return nullptr;
}

// ============================================================ //

BUFFER Buffer::create(const uint8_t* data, uint32_t size)
{
    BUFFER res(new Buffer());

    if (!res->init(data, size)) {

        return nullptr;
    }

    return res;
}

// ============================================================ //

Buffer::Buffer()
    : m_data(0),
      m_size(0)
{

}

// ============================================================ //

Buffer::~Buffer()
{
    release();
}

// ============================================================ //

bool Buffer::init(const uint8_t *data, uint32_t size)
{
    m_data = new uint8_t[size];

    if (!m_data) {

        return false;
    }

    m_size = size;

    clear();

    if (data) {

        write((void*)data, size);
    }

    return true;
}

// ============================================================ //

void Buffer::release()
{
    if (!empty()) {

        clear();

        delete[] m_data;
    }

    m_data = 0;

    m_size = 0;
}

// ============================================================ //

void Buffer::clear()
{
    if (!empty()) {

        memset(m_data, 0, m_size);
    }
}

// ============================================================ //

bool Buffer::read(void* data, uint32_t size, uint32_t offset)
{
    if (!m_data || !data || offset + size > m_size) {

        return false;
    }

    memcpy(data, m_data + offset, size);

    return true;
}

// ============================================================ //

bool Buffer::write(void* data, uint32_t size, uint32_t offset)
{
    if (!m_data || !data || offset + size > m_size) {

        return false;
    }

    memcpy(m_data + offset, data, size);

    return true;
}

// ============================================================ //

BUFFER Buffer::copy()
{
    if (!empty()) {

        return Buffer::create(m_data, m_size);
    }

    return nullptr;
}

// ============================================================ //

bool Buffer::empty() const
{
    return !(m_data || m_size);
}

// ============================================================ //

bool Buffer::equals(BUFFER buf) const
{
    if (!empty() && buf && !buf->empty() && buf->size() >= m_size) {

        return memcmp(m_data, buf->data(), m_size) == 0;
    }

    return false;
}


// ============================================================ //

uint8_t* Buffer::data() const
{
    return m_data;
}

// ============================================================ //

uint32_t Buffer::size() const
{
    return m_size;
}

// ============================================================ //

}
