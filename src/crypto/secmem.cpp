
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

#include "Zway/crypto/crypto.h"
#include "Zway/crypto/secmem.h"

#if defined _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

#include <memory.h>
#include <list>

namespace Zway {

SecMem* SecMem::instance = NULL;

// ============================================================ //

SecMem::SecMem()
    : m_lockedData(NULL),
      m_lockedSize(0)
{
}

// ============================================================ //

bool SecMem::setup(size_t size)
{
    if (instance) {

        return false;
    }

    instance = new SecMem();

    size_t pageSize = getPageSize();

    if (!size) {

#if defined _WIN32

        SIZE_T min, max;

        HANDLE hProcess = GetCurrentProcess();

        if (!GetProcessWorkingSetSize(hProcess, &min, &max)) {

            return false;
        }

        size = min;
#else

        struct rlimit rl;

        getrlimit(RLIMIT_MEMLOCK, &rl);

        size = rl.rlim_cur;

#endif

    }
    else
    if (size % pageSize) {

        return false;
    }

    instance->m_lockedData = new uint8_t[size];

    if (!instance->m_lockedData) {

        return false;
    }

    // lock pages

    size_t numLocks = size / pageSize;
    size_t pagesLocked = 0;

    for (size_t i=0; i<numLocks; ++i) {

#if defined _WIN32

        if (!VirtualLock((uint8_t*)instance->m_lockedData + i * pageSize, pageSize)) {

            break;
        }

#else

        if (mlock((uint8_t*)instance->m_lockedData + i * pageSize, pageSize)) {

            break;
        }

#endif

        pagesLocked++;
    }

    instance->m_lockedSize = pagesLocked * pageSize;

    return true;
}

// ============================================================ //

bool SecMem::cleanup()
{
    if (!instance) {

        return false;
    }

    if (instance->m_lockedData) {

        memset(instance->m_lockedData, 0, instance->m_lockedSize);

#if defined _WIN32

        VirtualUnlock(instance->m_lockedData, instance->m_lockedSize);
#else

        munlock(instance->m_lockedData, instance->m_lockedSize);

#endif

        delete[] instance->m_lockedData;
    }

    instance->m_lockedData = NULL;

    instance->m_lockedSize = 0;

    MutexLocker locker(instance->m_lockedMallocs);

    std::map<uint8_t*, size_t>& mallocs = instance->m_lockedMallocs;

    mallocs.clear();

    return true;
}

// ============================================================ //

uint8_t* SecMem::malloc(size_t size)
{
    if (!instance) {

        return NULL;
    }

    uint8_t* ptr = NULL;

    if (instance->m_lockedSize) {

        size_t offset = 0;

        if (instance->getOffset(size, &offset)) {

            ptr = instance->m_lockedData + offset;

            MutexLocker locker(instance->m_lockedMallocs);

            std::map<uint8_t*, size_t>& mallocs = instance->m_lockedMallocs;

            mallocs[ptr] = size;
        }
    }

    return ptr;
}

// ============================================================ //

bool SecMem::free(uint8_t *ptr)
{
    if (!instance) {

        return false;
    }

    if (instance->m_lockedSize) {

        MutexLocker locker(instance->m_lockedMallocs);

        std::map<uint8_t*, size_t>& mallocs = instance->m_lockedMallocs;

        if (mallocs.find(ptr) != mallocs.end()) {

            memset(ptr, 0, mallocs[ptr]);

            mallocs.erase(ptr);

            return true;
        }
    }

    return false;
}

// ============================================================ //

size_t SecMem::getPageSize()
{
#if defined _WIN32

    SYSTEM_INFO info;

    GetSystemInfo(&info);

    return info.dwPageSize;

#endif

#ifdef __gnu_linux__

    return getpagesize();

#endif

    return 0;
}

// ============================================================ //

size_t SecMem::getLockedSize()
{
    if (!instance) {

        return 0;
    }

    return instance->m_lockedSize;
}

// ============================================================ //

size_t SecMem::getLockedSizeUsed()
{
    if (!instance) {

        return 0;
    }

    size_t size = 0;

    MutexLocker locker(instance->m_lockedMallocs);

    for (auto &it : *instance->m_lockedMallocs) {

        size += it.second;
    }

    return size;
}

// ============================================================ //

size_t SecMem::getLockedSizeAvailable()
{
    return getLockedSize() - getLockedSizeUsed();
}

// ============================================================ //

bool SecMem::getOffset(size_t size, size_t* offset)
{
    size_t o = 0;
    size_t s = 0;

    MutexLocker locker(instance->m_lockedMallocs);

    for (auto &it : *instance->m_lockedMallocs) {

        size_t tmp = (size_t)it.first - (size_t)m_lockedData;

        if (tmp > o) {

            s = tmp - o;
        }
        else {

            s = 0;

            o = tmp + it.second;
        }

        if (size <= s) {

            *offset = o;

            return true;
        }
        else {

            s = 0;

            o = tmp + it.second;
        }
    }

    o += s;

    s = m_lockedSize - o;

    if (s && size <= s) {

        *offset = o;

        return true;
    }

    return false;
}

// ============================================================ //

}
