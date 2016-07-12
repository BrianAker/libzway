
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
#include "Zway/crypto/digest.h"
#include "Zway/crypto/secmem.h"

#include <string.h>
#include <nettle/md5.h>
#include <nettle/sha2.h>

namespace Zway { namespace Crypto {

// ============================================================ //

Digest::Digest(DigestType type)
    : m_type(type),
      m_ctx(0)
{
    uint32_t ctxSize = 0;

    switch (type) {

        case DIGEST_MD5:

            ctxSize = sizeof(md5_ctx);

            break;

        case DIGEST_SHA256:

            ctxSize = sizeof(sha256_ctx);

            break;
    }

    if (SecMem::getLockedSize()) {

        m_ctx = SecMem::malloc(ctxSize);
    }
    else {

        m_ctx = new uint8_t[ctxSize];
    }

    if (m_ctx) {

        switch (type) {

            case DIGEST_MD5:

                md5_init((md5_ctx*)m_ctx);

                break;

            case DIGEST_SHA256:

                sha256_init((sha256_ctx*)m_ctx);

                break;
        }
    }
}

// ============================================================ //

Digest::~Digest()
{
    if (m_ctx) {

        if (SecMem::getLockedSize()) {

            SecMem::free(m_ctx);
        }
        else {

            switch (m_type) {

                case DIGEST_MD5:

                    memset(m_ctx, 0, sizeof(md5_ctx));

                    break;

                case DIGEST_SHA256:

                    memset(m_ctx, 0, sizeof(sha256_ctx));

                    break;
            }

            delete[] m_ctx;
        }
    }
}

// ============================================================ //

void Digest::update(uint8_t* data, uint32_t size)
{
    if (m_ctx) {

        switch (m_type) {

            case DIGEST_MD5:

                md5_update((md5_ctx*)m_ctx, size, data);

                break;

            case DIGEST_SHA256:

                sha256_update((sha256_ctx*)m_ctx, size, data);

                break;
        }
    }
}

// ============================================================ //

void Digest::result(uint8_t* digest, uint32_t size)
{
    if (m_ctx) {

        switch (m_type) {

            case DIGEST_MD5:

                md5_digest((md5_ctx*)m_ctx, size, digest);

                break;

            case DIGEST_SHA256:

                sha256_digest((sha256_ctx*)m_ctx, size, digest);

                break;
        }
    }
}

// ============================================================ //

BUFFER Digest::digest(uint8_t *data, uint32_t size, Digest::DigestType type)
{
    BUFFER res;

    if (data && size) {

        Digest d(type);

        d.update(data, size);

        res = Buffer::create(nullptr, Digest::size(type));

        if (res) {

            d.result(res->data(), res->size());
        }
    }

    return res;
}

// ============================================================ //

BUFFER Digest::digest(BUFFER data, DigestType type)
{
    BUFFER res;

    if (data) {

        res = digest(data->data(), data->size(), type);
    }

    return res;
}

// ============================================================ //

std::string Digest::digestHexStr(uint8_t *data, uint32_t size, Digest::DigestType type)
{
    std::string res;

    BUFFER buf = digest(data, size, type);

    if (buf) {

        res = hexStr(buf);
    }

    return res;
}

// ============================================================ //

std::string Digest::digestHexStr(BUFFER data, Digest::DigestType type)
{
    std::string res;

    if (data) {

        res = digestHexStr(data->data(), data->size(), type);
    }

    return res;
}

// ============================================================ //

uint32_t Digest::size(Digest::DigestType type)
{
    switch (type) {

        case DIGEST_MD5:

            return DIGEST_MD5_SIZE;

        case DIGEST_SHA256:

            return DIGEST_SHA256_SIZE;
    }

    return 0;
}

// ============================================================ //

}

}
