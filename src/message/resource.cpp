
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

#include "Zway/message/resource.h"
#include "Zway/crypto/digest.h"
#include <fstream>

namespace Zway {

// ============================================================ //
// Resource
// ============================================================ //

Resource::Resource()
    : UBJ::Object()
{

}

// ============================================================ //

void Resource::updateMD5()
{
    if (m_data) {

        m_md5 = Crypto::Digest::digest(m_data);
    }
}

// ============================================================ //

RESOURCE Resource::create()
{
    RESOURCE p(new Resource());

    return p;
}

// ============================================================ //

RESOURCE Resource::createFromData(const std::string& name, const uint8_t* data, uint32_t size, Type type)
{
    if (!size) {

        return nullptr;
    }

    RESOURCE p(new Resource());

    BUFFER buffer = Buffer::create(data, size);

    p->setType(type);

    p->setName(name);

    p->setData(buffer);

    p->updateMD5();

    return p;
}

// ============================================================ //

RESOURCE Resource::createFromFile(const std::string& name, const std::string& filename, Type type)
{
    std::ifstream ifs(filename.c_str(), std::ios::binary);

    if (!ifs) {

        return nullptr;
    }

    ifs.seekg(0, std::ios_base::end);

    uint32_t size = ifs.tellg();

    ifs.seekg(0, std::ios_base::beg);

    RESOURCE p = RESOURCE(new Resource());

    BUFFER buffer = Buffer::create(nullptr, size);

    ifs.read((char*)buffer->data(), size);

    p->setType(type);

    p->setName(name);

    p->setData(buffer);

    p->updateMD5();

    return p;
}

// ============================================================ //

RESOURCE Resource::createFromText(const std::string& name, const std::string& text)
{
    return createFromData(name, (uint8_t*)text.c_str(), text.size(), Zway::Resource::TextType);
}

// ============================================================ //

void Resource::setId(uint32_t id)
{
    setField("id", id);
}

// ============================================================ //

void Resource::setType(Type type)
{
    setField("type", type);
}

// ============================================================ //

void Resource::setName(const std::string& name)
{
    setField("name", name);
}

// ============================================================ //

bool Resource::setData(const uint8_t* data, uint32_t size)
{
    m_data = Buffer::create(data, size);

    updateMD5();

    return true;
}

// ============================================================ //

bool Resource::setData(BUFFER data)
{
    m_data = data;

    updateMD5();

    return true;
}

// ============================================================ //

bool Resource::saveToFile(const std::string& filename)
{
    if (!m_data) {

        return false;
    }

    std::ofstream ofs(filename.c_str());

    if (!ofs) {

        return false;
    }

    ofs.write((char*)m_data->data(), m_data->size());

    return true;
}

// ============================================================ //

std::string Resource::toString()
{
    if (m_data) {

        return std::string((char*)m_data->data(), m_data->size());
    }

    return std::string();
}

// ============================================================ //

uint32_t Resource::id()
{
    return value("id").toInt();
}

// ============================================================ //

Resource::Type Resource::type()
{
    return (Resource::Type)value("type").toInt();
}

// ============================================================ //

uint32_t Resource::size()
{
    if (m_data) {

        return m_data->size();
    }

    return 0;
}

// ============================================================ //

std::string Resource::name()
{
    return value("name").toString();
}

// ============================================================ //

BUFFER Resource::data()
{
    return m_data;
}

// ============================================================ //

BUFFER Resource::md5()
{
    return m_md5;
}

// ============================================================ //

std::mutex &Resource::mutex()
{
    return m_mutex;
}

// ============================================================ //

}
