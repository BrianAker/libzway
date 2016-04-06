
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

#include "Zway/util/exif.h"

namespace Zway {

// ============================================================ //

/**
 * @brief Exif::Exif
 */

Exif::Exif()
    : m_data(nullptr)
{

}

/**
 * @brief Exif::Exif
 * @param path
 */

Exif::Exif(const std::string &path)
    : m_data(nullptr)
{
    load(path);
}

/**
 * @brief Exif::Exif
 * @param data
 * @param size
 */

Exif::Exif(const uint8_t *data, uint32_t size)
    : m_data(nullptr)
{
    load(data, size);
}

/**
 * @brief Exif::~Exif
 */

Exif::~Exif()
{
    clear();
}

/**
 * @brief Exif::load
 * @param path
 * @return
 */

bool Exif::load(const std::string &path)
{
    clear();

    // load exif data

    m_data = exif_data_new_from_file(path.c_str());

    if (m_data) {

        //exif_data_dump(m_data);

        return true;
    }

    return false;
}

/**
 * @brief Exif::load
 * @param data
 * @param size
 * @return
 */

bool Exif::load(const uint8_t *data, uint32_t size)
{
    clear();

    // load exif data

    m_data = exif_data_new_from_data(data, size);

    if (m_data) {

        //exif_data_dump(m_data);

        return true;
    }

    return false;
}

/**
 * @brief Exif::clear
 */

void Exif::clear()
{
    if (m_data) {

        exif_data_unref(m_data);

        m_data = nullptr;
    }
}

/**
 * @brief Exif::isValid
 * @return
 */

bool Exif::isValid()
{
    return m_data;
}

/**
 * @brief Exif::getShortValue
 * @param tag
 * @return
 */

uint16_t Exif::getShortValue(ExifTag tag)
{
    if (m_data) {

        ExifEntry *entry = getEntry(tag);

        if (entry && entry->size == 2) {

            return (uint16_t)*entry->data;
        }
    }

    return 0;
}

/**
 * @brief Exif::getEntry
 * @param tag
 * @return
 */

ExifEntry *Exif::getEntry(ExifTag tag)
{
    if (m_data) {

        ExifEntry *entry = exif_content_get_entry(((::ExifData*)m_data)->ifd[EXIF_IFD_0], tag);

        // check all ifd's ??

        return entry;
    }

    return nullptr;
}

// ============================================================ //

}
