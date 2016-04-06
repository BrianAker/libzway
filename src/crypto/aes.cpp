
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

#include "Zway/crypto/aes.h"
#include "Zway/crypto/secmem.h"

#include <string.h>
#include <nettle/aes.h>
#include <nettle/ctr.h>

namespace Zway {

namespace Crypto {

// ============================================================ //
// AES
// ============================================================ //

typedef struct CTR_CTX(aes_ctx, AES_BLOCK_SIZE) AES_CTR_CTX;

// ============================================================ //

AES::AES()
{
    /*
    if (SecMem::getLockedSize()) {

        m_ctx = SecMem::malloc(sizeof(AES_CTR_CTX));
    }
    else {
    */
        m_ctx = new uint8_t[sizeof(AES_CTR_CTX)];
    /*
    }
    */
}

// ============================================================ //

AES::~AES()
{
    if (m_ctx) {

        /*
        if (SecMem::getLockedSize()) {

            SecMem::free(m_ctx);
        }
        else {
        */
            memset(m_ctx, 0, sizeof(AES_CTR_CTX));

            delete[] m_ctx;
        /*
        }
        */

        m_ctx = 0;
    }
}

// ============================================================ //

//! Set key
/*!
 * \param key           The key
 */

void AES::setKey(BUFFER key)
{
    if (key) {

        aes_set_encrypt_key(&((AES_CTR_CTX*)m_ctx)->ctx, AES_KEY_SIZE, key->data());
    }
}

// ============================================================ //

//! Set counter
/*!
 * \param ctr           The counter
 */

void AES::setCtr(BUFFER ctr)
{
    if (ctr) {

        CTR_SET_COUNTER((AES_CTR_CTX*)m_ctx, ctr->data());
    }
}

// ============================================================ //

void AES::encrypt(void *src, void *dst, uint32_t size)
{
    /*
    cbc_encrypt(
            &m_ctx,
            (nettle_crypt_func*)aes_encrypt,
            AES_BLOCK_SIZE,
            m_ctx.iv,
            size,
            data,
            data);
    */

    uint32_t blockSize = 1024;

    uint32_t bytesEncrypted = 0;

    while (bytesEncrypted < size) {

        uint32_t bytesToEncrypt = blockSize;

        if (size - bytesEncrypted < blockSize) {

            bytesToEncrypt = size - bytesEncrypted;
        }

        ctr_crypt(
                &((AES_CTR_CTX*)m_ctx)->ctx,
                (nettle_cipher_func*)aes_encrypt,
                AES_BLOCK_SIZE,
                ((AES_CTR_CTX*)m_ctx)->ctr,
                bytesToEncrypt,
                (uint8_t*)dst + bytesEncrypted,
                (uint8_t*)src + bytesEncrypted);

        bytesEncrypted += bytesToEncrypt;
    }
}

// ============================================================ //

//! Encrypt data
/*!
 * \param data          Buffer to encrypt
 * \param size          Buffer size
 */

void AES::encrypt(BUFFER src, BUFFER dst, uint32_t size)
{
    encrypt(src->data(), dst->data(), size);
}

// ============================================================ //

void AES::decrypt(void *src, void *dst, uint32_t size)
{
    /*
    cbc_decrypt(
            &m_ctx,
            (nettle_crypt_func*)aes_decrypt,
            AES_BLOCK_SIZE,
            m_ctx.iv,
            size,
            data,
            data);
    */

    uint32_t blockSize = 1024;

    uint32_t bytesDecrypted = 0;

    while (bytesDecrypted < size) {

        uint32_t bytesToDecrypt = blockSize;

        if (size - bytesDecrypted < blockSize) {

            bytesToDecrypt = size - bytesDecrypted;
        }

        ctr_crypt(
                &((AES_CTR_CTX*)m_ctx)->ctx,
                (nettle_cipher_func*)aes_encrypt,
                AES_BLOCK_SIZE,
                ((AES_CTR_CTX*)m_ctx)->ctr,
                bytesToDecrypt,
                (uint8_t*)dst + bytesDecrypted,
                (uint8_t*)src + bytesDecrypted);

        bytesDecrypted += bytesToDecrypt;
    }
}

// ============================================================ //

//! Decrypt data
/*!
 * \param data          Buffer to decrypt
 * \param size          Buffer size
 */

void AES::decrypt(BUFFER src, BUFFER dst, uint32_t size)
{
    decrypt(src->data(), dst->data(), size);
}

// ============================================================ //

}

}
