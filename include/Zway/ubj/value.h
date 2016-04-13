
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

#ifndef UBJ_VALUE_H_
#define UBJ_VALUE_H_

#include "Zway/ubj/ubj.h"
#include "Zway/buffer.h"

#include <string>
#include <map>

namespace Zway { namespace UBJ {

class Value;

typedef std::map<std::string, Value> ValueMap;

class Value
{
public:

    class Reader
    {
    public:

        static bool read(Value &val, const Zway::BUFFER &buf);

        static bool read(Value &val, const uint8_t *data, size_t size);

    private:

        static void readDynamic(Value &val, ubjr_dynamic_t &dyn);

        static void readObject(Value &val, ubjr_object_t &obj);

        static void readArray(Value &val, ubjr_array_t &arr);
    };

    class Writer
    {
    public:

        static Zway::BUFFER write(const Value &val);

    private:

        static void writeObject(const Value &val, ubjw_context_t *ctx);

        static void writeArray(const Value &val, ubjw_context_t *ctx);

        static void writeValue(const std::string &key, const Value &val, ubjw_context_t *ctx);
    };

public:

    Value();

    Value(const std::string& str);

    Value(const char* str, size_t len = 0);

    Value(int32_t val);

  //Value(bool val);

    Value(Zway::BUFFER buf);


    Value clone() const;

    Value &value(const std::string& key);

    Value const operator[](const std::string &key) const;

    Value const operator[](size_t index) const;

    Value &operator[](const std::string &key);

    Value &operator[](size_t index);

    Value &operator<<(const Value &val);

    void push(const Value &val);

    void clear();


    bool toBool(bool def = false) const;

    int32_t toInt(int32_t def = 0) const;

    std::string toString() const;


    void setField(const std::string& key, const UBJ::Value &val);


    bool hasField(const std::string &key) const;

    bool remove(const std::string &key);


    UBJ_TYPE type() const;

    bool isObject() const;

    bool isArray() const;

    bool isValid() const;


    ValueMap &values();

    size_t numValues() const;

    Zway::BUFFER buffer() const;

    uint8_t *data() const;

    size_t size() const;

    ValueMap::const_iterator cbegin() const;

    ValueMap::const_iterator cend() const;

    std::string dump(int indent = 2, int depth = 0) const;

protected:

    void setData(const uint8_t* data, size_t size, UBJ_TYPE type);

    void setString(const char *str, size_t len);

protected:

    UBJ_TYPE m_type;

    ValueMap m_values;

    Zway::BUFFER m_buffer;

    std::string m_currentKey;
};

class Object : public Value
{
public:

    Object()
    {
        m_type = UBJ_OBJECT;
    }

    Object(const Value &val)
    {
        *((Value*)this) = val;

        m_type = UBJ_OBJECT;
    }
};

#define UBJ_OBJ(x) (Zway::UBJ::Object() << x)

class Array : public Value
{
public:

    Array()
    {
        m_type = UBJ_ARRAY;
    }

    Array(const Value &val)
    {
        *((Value*)this) = val;

        m_type = UBJ_ARRAY;
    }
};

#define UBJ_ARR(x) (Zway::UBJ::Array() << x)

}}

#endif
