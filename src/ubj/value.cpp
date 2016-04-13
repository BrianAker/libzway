
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

#include "Zway/ubj/value.h"

#include <cstring>
#include <sstream>
#include <vector>

namespace Zway { namespace UBJ {

bool Value::Reader::read(Value &val, const Zway::BUFFER &buf)
{
    return read(val, buf->data(), buf->size());
}

bool Value::Reader::read(Value &val, const uint8_t *data, size_t size)
{
    ubjr_context_t* ctx = ubjr_open_memory(data, data + size);

    ubjr_dynamic_t d =  ubjr_read_dynamic(ctx);

    if (d.type != UBJ_OBJECT && d.type != UBJ_ARRAY) {

        ubjr_close_context(ctx);

        return false;
    }

    readDynamic(val, d);

    ubjr_close_context(ctx);

    return true;
}

void Value::Reader::readDynamic(Value &val, ubjr_dynamic_t &dyn)
{
    switch (dyn.type) {

        case UBJ_INT32: {

            int32_t v = dyn.integer;

            val.setData((uint8_t*)&v, sizeof(v), dyn.type);

            break;
        }

        case UBJ_STRING: {

            std::string v(dyn.string);

            val.setData((uint8_t*)v.c_str(), v.length() + 1, dyn.type);

            break;
        }

        case UBJ_OBJECT:

            val.m_type = dyn.type;

            readObject(val, dyn.container_object);

            break;

        case UBJ_ARRAY:

            val.m_type = dyn.type;

            readArray(val, dyn.container_array);

            break;

        default:

            break;
    }
}

void Value::Reader::readObject(Value &val, ubjr_object_t &obj)
{
    for (uint32_t i=0; i<obj.size; i++) {

        ubjr_string_t k = obj.keys[i];

        ubjr_dynamic_t d = ubjr_object_lookup(&obj, k);

        readDynamic(val[k], d);
    }
}

void Value::Reader::readArray(Value &val, ubjr_array_t &arr)
{
    if (arr.type == UBJ_MIXED) {

        for (uint32_t i=0; i<arr.size; i++) {

            ubjr_dynamic_t* dyn = (ubjr_dynamic_t*)((uint8_t*)arr.values + i * ubjr_local_type_size(arr.type));

            std::stringstream ss;

            ss << i;

            readDynamic(val[ss.str()], *dyn);
        }
    }
    else
    if (arr.type == UBJ_INT8) {

        val.m_buffer = Zway::Buffer::create((uint8_t*)arr.values, arr.size);
    }
}



Zway::BUFFER Value::Writer::write(const Value &val)
{
    std::vector<uint8_t> buf;

    auto writeCb = [] (const void* data, size_t size, size_t count, void* userdata) -> size_t {

        std::vector<uint8_t>* buf = (std::vector<uint8_t>*)userdata;

        buf->insert(buf->end(), (uint8_t*)data, (uint8_t*)data + size * count);

        return size * count;
    };

    auto closeCb = [] (void* /*userdata*/) -> int {

        return 0;
    };

    auto errorCb = [] (const char* /*error_msg*/) {

    };

    ubjw_context_t* ctx = ubjw_open_callback(&buf, writeCb, closeCb, errorCb);

    if (!ctx) {

        return nullptr;
    }

    buf.reserve(1024);

    if (val.m_type == UBJ_OBJECT) {

        writeObject(val, ctx);
    }
    else
    if (val.m_type == UBJ_ARRAY) {

        writeArray(val, ctx);
    }

    return Zway::Buffer::create(&buf[0], buf.size());
}

void Value::Writer::writeObject(const Value &val, ubjw_context_t *ctx)
{
    ubjw_begin_object(ctx, UBJ_MIXED, val.numValues());

    for (auto it = val.cbegin(); it != val.cend(); ++it) {

        writeValue(it->first, it->second, ctx);
    }

    ubjw_end(ctx);
}

void Value::Writer::writeArray(const Value &val, ubjw_context_t *ctx)
{
    if (!val.size()) {

        ubjw_begin_array(ctx, UBJ_MIXED, val.numValues());

        for (auto it = val.cbegin(); it != val.cend(); ++it) {

            writeValue(std::string(), it->second, ctx);
        }

        ubjw_end(ctx);
    }
    else {

        ubjw_write_buffer(ctx, val.m_buffer->data(), UBJ_INT8, val.m_buffer->size());
    }
}

void Value::Writer::writeValue(const std::string &key, const Value &val, ubjw_context_t *ctx)
{
    if (!key.empty()) {

        ubjw_write_key(ctx, key.c_str());
    }

    switch (val.m_type) {

        case UBJ_INT32: {

            ubjw_write_int32(ctx, val.toInt());

            break;
        }

        case UBJ_STRING: {

            ubjw_write_string(ctx, val.toString().c_str());

            break;
        }

        case UBJ_OBJECT:

            writeObject(val, ctx);

            break;

        case UBJ_ARRAY:

            writeArray(val, ctx);

            break;

        default:

            ubjw_write_null(ctx);

            break;
    }
}

Value::Value()
    : m_type(UBJ_NULLTYPE)
{

}

Value::Value(const std::string& str)
    : m_type(UBJ_NULLTYPE)
{
    setString(str.c_str(), str.size());
}

Value::Value(const char* str, size_t len)
    : m_type(UBJ_NULLTYPE)
{
    setString(str, len);
}

Value::Value(int32_t val)
    : m_type(UBJ_NULLTYPE)
{
    setData((uint8_t*)&val, sizeof(int32_t), UBJ_INT32);
}

/*
Value::Value(bool val)
    : m_type(val ? UBJ_BOOL_TRUE : UBJ_BOOL_FALSE)
{

}
*/

Value::Value(Zway::BUFFER buf)
    : m_type(UBJ_NULLTYPE)
{
    m_type = UBJ_ARRAY;

    m_buffer = buf;
}

Value Value::clone() const
{
    Value res;

    if (m_buffer) {

        res = Value(m_buffer->copy());
    }

    res.m_type = m_type;

    for (auto &it : m_values) {

        res[it.first] = it.second.clone();
    }

    return res;
}

Value &Value::value(const std::string &key)
{
    return (*this)[key];
}

Value const Value::operator[](const std::string &key) const
{
    if (m_values.find(key) != m_values.end()) {

        return m_values.at(key).clone();
    }

    return Value();
}

Value const Value::operator[](size_t index) const
{
    std::stringstream ss;

    ss << index;

    return (*this)[ss.str()];
}

Value& Value::operator[](const std::string &key)
{
    return m_values[key];
}

Value& Value::operator[](size_t index)
{
    std::stringstream ss;

    ss << index;

    return m_values[ss.str()];
}

Value &Value::operator<<(const Value &val)
{
    if (m_type == UBJ_OBJECT || !m_type) {

        if (m_currentKey.empty() && val.m_type == UBJ_STRING) {

            m_currentKey = val.toString();
        }
        else {

            m_values[m_currentKey] = val;

            m_currentKey.clear();
        }
    }
    else
    if (m_type == UBJ_ARRAY) {

        push(val);
    }

    return *this;
}

void Value::push(const Value &val)
{
    // TODO: this must be improved

    std::stringstream ss;

    ss << m_values.size();

    (*this)[ss.str()] = val;
}

void Value::clear()
{
    m_buffer.reset();

    m_values.clear();
}

bool Value::toBool(bool def) const
{
    if (m_type == UBJ_BOOL_TRUE) {

        return true;
    }
    else
    if (m_type == UBJ_BOOL_FALSE) {

        return false;
    }

    return def;
}

int32_t Value::toInt(int32_t def) const
{
    if (m_type == UBJ_INT32) {

        return *((int32_t*)m_buffer->data());
    }

    return def;
}

std::string Value::toString() const
{
    if (m_type == UBJ_STRING) {

        return std::string((char*)m_buffer->data());
    }

    return std::string();
}

void Value::setField(const std::string &key, const Value &val)
{
    (*this)[key] = val;
}

bool Value::hasField(const std::string &key) const
{
    return m_values.find(key) != m_values.end();
}

bool Value::remove(const std::string &key)
{
    if (hasField(key)) {

        m_values.erase(m_values.find(key));

        return true;
    }

    return false;
}

UBJ_TYPE Value::type() const
{
    return m_type;
}

bool Value::isObject() const
{
    return m_type == UBJ_OBJECT;
}

bool Value::isArray() const
{
    return m_type == UBJ_ARRAY;
}

bool Value::isValid() const
{
    return (m_buffer && m_buffer->size()) || !m_values.empty();
}

ValueMap &Value::values()
{
    return m_values;
}

size_t Value::numValues() const
{
    return m_values.size();
}

Zway::BUFFER Value::buffer() const
{
    return m_buffer;
}

uint8_t *Value::data() const
{
    if (m_buffer) {

        return m_buffer->data();
    }

    return nullptr;
}

size_t Value::size() const
{
    if (m_buffer) {

        return m_buffer->size();
    }

    return 0;
}

ValueMap::const_iterator Value::cbegin() const
{
    return m_values.cbegin();
}

ValueMap::const_iterator Value::cend() const
{
    return m_values.cend();
}

std::string Value::dump(int indent, int depth) const
{
    std::stringstream ss;

    std::string ind = std::string(indent * depth, ' ');

    if (m_type == UBJ_OBJECT) {

        if (!m_values.empty()) {

            ss << "{\n";

            for (auto it = m_values.cbegin(); it != m_values.cend(); ++it) {

                ss << std::string(indent * (depth+1), ' ') << it->first << " : " << it->second.dump(indent, depth + 1) << "\n";
            }

            ss << ind << "}";
        }
        else {

            ss << "{}";
        }
    }
    if (m_type == UBJ_ARRAY) {

        if (!m_buffer) {

            if (!m_values.empty()) {

                ss << "[\n";

                for (auto it = m_values.cbegin(); it != m_values.cend();) {

                    ss << std::string(indent * (depth+1), ' ') << it->second.dump(indent, depth + 1);

                    ss << (++it == m_values.cend() ? ' ' : ',') << "\n";
                }

                ss << ind << "]";
            }
            else {

                ss << "[]";
            }
        }
        else {

            ss << "DATA " << size();
        }
    }
    else
    if (m_type == UBJ_BOOL_TRUE) {

        ss << "TRUE";
    }
    else
    if (m_type == UBJ_BOOL_FALSE) {

        ss << "FALSE";
    }
    else
    if (m_type == UBJ_INT32) {

        ss << toInt();
    }
    else
    if (m_type == UBJ_STRING) {

        ss << '"' << toString() << '"';
    }
    else
    if (m_type == UBJ_NULLTYPE) {

        ss << "NULL";
    }

    return ss.str();
}

void Value::setData(const uint8_t* data, size_t size, UBJ_TYPE type)
{
    if (type == UBJ_STRING) {

        setString((char*)data, size);
    }
    else {

        m_type = type;

        m_buffer = Zway::Buffer::create(data, size);
    }
}

void Value::setString(const char* str, size_t len)
{
    m_type = UBJ_STRING;

    size_t l = len > 0 ? len : strlen(str);

    m_buffer = Zway::Buffer::create(nullptr, l + 1);

    m_buffer->write((uint8_t*)str, l);
}

}}
