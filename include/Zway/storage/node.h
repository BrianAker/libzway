
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

#ifndef STORAGE_NODE_H_
#define STORAGE_NODE_H_

namespace Zway {

// ============================================================ //

class Storage::Node
{
public:

    enum NodeType {

        UnknownType,

        RequestType,

        ContactType,

        HistoryType,

        MessageType,

        ResourceType,

        DirectoryType,

        CustomType,
    };

    typedef std::shared_ptr<Storage::Node> Pointer;

    static Pointer create(uint32_t type=0, uint32_t parent=0, const std::string& name=std::string());

    virtual ~Node();

    Pointer copy();

    uint32_t id() const;

    uint32_t type() const;

    uint32_t parent() const;

    std::string name() const;

    uint32_t user1() const;

    uint32_t user2() const;

    std::string user3() const;

    std::string user4() const;

    BUFFER head();

    BUFFER body();

    bool headUbj(UBJ::Value &head, bool secure = false);

    bool bodyUbj(UBJ::Value &body, bool secure = false);

    void setId(uint32_t id);

    void setType(uint32_t type);

    void setParent(uint32_t parent);

    void setName(const std::string& name);

    void setUser1(uint32_t val);

    void setUser2(uint32_t val);

    void setUser3(const std::string& val);

    void setUser4(const std::string& val);

    bool setHead(const uint8_t* data, uint32_t size);

    bool setHead(BUFFER buffer);

    bool setBody(const uint8_t* data, uint32_t size);

    bool setBody(BUFFER buffer);

    bool setHeadUbj(const UBJ::Value &obj);

    bool setBodyUbj(const UBJ::Value &obj);

protected:

    Node(uint32_t type=0, uint32_t parent=0, const std::string& name=std::string());

protected:

    uint32_t m_id;

    uint32_t m_type;

    uint32_t m_parent;

    std::string m_name;

    uint32_t m_user1;

    uint32_t m_user2;

    std::string m_user3;

    std::string m_user4;

    BUFFER m_head;

    BUFFER m_body;
};

// ============================================================ //

}

#endif /* STORAGE_NODE_H_ */
