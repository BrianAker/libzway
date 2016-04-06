
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

#ifndef RESOURCE_H_
#define RESOURCE_H_

#include "Zway/ubj/value.h"
#include "Zway/thread.h"

#include <deque>

namespace Zway {

// ============================================================ //

class Resource : public UBJ::Object, public EnableLock<Resource>
{
public:

    enum Type{

        UnknownType,

        TextType,

        FileType,

        ImageType,

        AudioType,

        VideoType
    };

    enum Status {

        UnknownStatus
    };

    typedef std::shared_ptr<Resource> Pointer;

    static Pointer create();

    static Pointer createFromData(const std::string& name, const uint8_t* data, uint32_t size, Type type);

    static Pointer createFromFile(const std::string& name, const std::string& filename, Type type);

    static Pointer createFromText(const std::string& name, const std::string& text);

    void setId(uint32_t id);

    void setType(Type type);

    void setName(const std::string& name);

    bool setData(const uint8_t* data, uint32_t size);

    bool setData(BUFFER data);

    bool saveToFile(const std::string& filename);

    std::string toString();

    uint32_t id();

    Type type();

    uint32_t size();

    std::string name();

    BUFFER data();

    BUFFER md5();

    std::string md5Hex();

    std::mutex &mutex();

protected:

    Resource();

    void updateMD5();

protected:

    BUFFER m_data;

    BUFFER m_md5;

    std::mutex m_mutex;
};

typedef Resource::Pointer RESOURCE;

typedef std::deque<RESOURCE> RESOURCE_LIST;

typedef std::map<uint32_t, RESOURCE> RESOURCE_MAP;

// ============================================================ //

}

#endif /* RESOURCE_H_ */
