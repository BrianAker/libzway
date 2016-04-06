
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

#ifndef SECMEM_H
#define SECMEM_H

#include <Zway/buffer.h>
#include <Zway/thread.h>
#include <stddef.h>
#include <map>

namespace Zway {

// ============================================================ //

class SecMem
{
public:

    static bool setup(size_t size=0);

    static bool cleanup();

    static uint8_t *malloc(size_t size);

    static bool free(uint8_t* ptr);

    static size_t getPageSize();

    static size_t getLockedSize();

    static size_t getLockedSizeUsed();

    static size_t getLockedSizeAvailable();

private:

    SecMem();

    bool getOffset(size_t size, size_t* offset);

private:

    uint8_t* m_lockedData;

    size_t m_lockedSize;

    ThreadSafe<std::map<uint8_t*, size_t> > m_lockedMallocs;

    static SecMem* instance;
};

// ============================================================ //

}

#endif // SECMEM_H
