
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
#include "Zway/crypto/random.h"
#include "Zway/crypto/rsa.h"

#include <nettle/yarrow.h>
#include <nettle/rsa.h>

namespace Zway { namespace Crypto {

// ============================================================ //
// RSA
// ============================================================ //

//! Create a key pair
/*!
 *  Key size would be 1024 or 2048
 */
/*!
 * \param publicKey     String to receive the public key
 * \param privateKey    String to receive the private key
 * \param bits          Key size in bits
 */

void mpzFromHexStr(const std::string& str, mpz_t* z)
{
    mpz_init(*z);

    mpz_set_str(*z, str.c_str(), 16);
}

// ============================================================ //

std::string mpzToHexStr(mpz_t z)
{
    std::string res;

    uint32_t len = mpz_sizeinbase(z, 16) + 1;

    res.resize(len);

    mpz_get_str(&res[0], 16, z);

    return res;
}

// ============================================================ //

UBJ::Value publicKeyToUbj(struct rsa_public_key& publicKey)
{
    UBJ::Object res;

    res["e"] = mpzToHexStr(publicKey.e);

    res["n"] = mpzToHexStr(publicKey.n);

    res["s"] = publicKey.size;

    return res;
}

// ============================================================ //

UBJ::Value privateKeyToUbj(struct rsa_private_key& privateKey)
{
    UBJ::Object res;

    res["a"] = mpzToHexStr(privateKey.a);

    res["b"] = mpzToHexStr(privateKey.b);

    res["c"] = mpzToHexStr(privateKey.c);

    res["d"] = mpzToHexStr(privateKey.d);

    res["p"] = mpzToHexStr(privateKey.p);

    res["q"] = mpzToHexStr(privateKey.q);

    res["s"] = privateKey.size;

    return res;
}

// ============================================================ //

void ubjToPublicKey(UBJ::Value publicKeyObj, struct rsa_public_key& publicKey)
{
    mpzFromHexStr(publicKeyObj["e"].toString(), &publicKey.e);

    mpzFromHexStr(publicKeyObj["n"].toString(), &publicKey.n);

    publicKey.size = publicKeyObj["s"].toInt();
}

// ============================================================ //

void ubjToPrivateKey(UBJ::Value privateKeyObj, struct rsa_private_key& privateKey)
{
    mpzFromHexStr(privateKeyObj["a"].toString(), &privateKey.a);

    mpzFromHexStr(privateKeyObj["b"].toString(), &privateKey.b);

    mpzFromHexStr(privateKeyObj["c"].toString(), &privateKey.c);

    mpzFromHexStr(privateKeyObj["d"].toString(), &privateKey.d);

    mpzFromHexStr(privateKeyObj["p"].toString(), &privateKey.p);

    mpzFromHexStr(privateKeyObj["q"].toString(), &privateKey.q);

    privateKey.size = privateKeyObj["s"].toInt();
}

// ============================================================ //

bool RSA::createKeyPair(
        UBJ::Value& publicKeyObj,
        UBJ::Value& privateKeyObj,
        uint32_t bits)
{
    struct rsa_public_key publicKey;

    struct rsa_private_key privateKey;

    rsa_public_key_init(&publicKey);

    rsa_private_key_init(&privateKey);

    if (!rsa_generate_keypair(
            &publicKey,
            &privateKey,
            Random::getYarrowCtx(),
            (nettle_random_func*)yarrow256_random,
            NULL,
            NULL,
            bits,
            30)) {

        rsa_public_key_clear(&publicKey);

        rsa_private_key_clear(&privateKey);

        return false;
    }

    publicKeyObj = publicKeyToUbj(publicKey);

    privateKeyObj = privateKeyToUbj(privateKey);

    rsa_public_key_clear(&publicKey);

    rsa_private_key_clear(&privateKey);

    return true;
}

// ============================================================ //

//! Encrypt input buffer to new allocated ouput buffer
/*!
 * \param publicKey     Public key to use for encryption
 * \param src           Input buffer
 * \param srcSize       Size of input buffer in uint8_ts
 * \param dst           Pointer to receive the output buffer
 * \param dstSize       Pointer to receive the output buffer size
 */

BUFFER RSA::encrypt(
        UBJ::Value &publicKeyObj,
        BUFFER buf)
{
    struct rsa_public_key publicKey;

    ubjToPublicKey(publicKeyObj, publicKey);

    mpz_t z;

    mpz_init(z);

    if (!rsa_encrypt(
            &publicKey,
            Random::getYarrowCtx(),
            (nettle_random_func*)yarrow256_random,
            buf->size(),
            buf->data(),
            z)) {

        return BUFFER();
    }

    uint32_t len = mpz_sizeinbase(z, 16) + 1;

    BUFFER res = Buffer::create(NULL, len);

    if (!res) {

        return BUFFER();
    }

    mpz_get_str((char*)res->data(), 16, z);

    mpz_clear(z);

    rsa_public_key_clear(&publicKey);

    return res;
}

// ============================================================ //

//! Decrypt input buffer to new allocated ouput buffer
/*!
 * \param privateKey    Key to use for decryption
 * \param src           Input buffer
 * \param srcSize       Input buffer size
 * \param dst           Pointer to receive the output buffer
 * \param dstSize       Pointer to receive the output buffer size
 */

BUFFER RSA::decrypt(UBJ::Value &privateKeyObj, BUFFER buf)
{
    struct rsa_private_key privateKey;

    ubjToPrivateKey(privateKeyObj, privateKey);

    mpz_t z;

    mpz_init(z);

    mpz_set_str(z, (char*)buf->data(), 16);

    BUFFER tmp = Buffer::create(NULL, 2048);

    size_t len = tmp->size();

    if (!rsa_decrypt(
            &privateKey,
            &len,
            tmp->data(),
            z)) {

        return BUFFER();
    }

    BUFFER res = Buffer::create(tmp->data(), len);

    if (!res) {

        return BUFFER();
    }

    mpz_clear(z);

    rsa_private_key_clear(&privateKey);

    return res;
}

// ============================================================ //

BUFFER RSA::sign(UBJ::Value &privateKeyObj, BUFFER buf)
{
    struct rsa_private_key privateKey;

    ubjToPrivateKey(privateKeyObj, privateKey);

    BUFFER digest = Digest::digest(buf, Digest::DIGEST_SHA256);

    mpz_t z;

    mpz_init(z);

    if (!rsa_sha256_sign_digest(&privateKey, digest->data(), z)) {

        mpz_clear(z);

        rsa_private_key_clear(&privateKey);

        return BUFFER();
    }

    uint32_t len = mpz_sizeinbase(z, 16) + 1;

    BUFFER sign = Buffer::create(NULL, len);

    if (!sign) {

        mpz_clear(z);

        rsa_private_key_clear(&privateKey);

        return BUFFER();
    }

    mpz_get_str((char*)sign->data(), 16, z);

    mpz_clear(z);

    rsa_private_key_clear(&privateKey);

    return sign;
}

// ============================================================ //

bool RSA::verify(
        UBJ::Value &publicKeyObj,
        BUFFER buf,
        BUFFER sign)
{
    struct rsa_public_key publicKey;

    ubjToPublicKey(publicKeyObj, publicKey);

    BUFFER digest = Digest::digest(buf, Digest::DIGEST_SHA256);

    mpz_t z;

    mpz_init(z);

    mpz_set_str(z, (char*)sign->data(), 16);

    if (!rsa_sha256_verify_digest(&publicKey, digest->data(), z)) {

        mpz_clear(z);

        rsa_public_key_clear(&publicKey);

        return false;
    }

    mpz_clear(z);

    rsa_public_key_clear(&publicKey);

    return true;
}

// ============================================================ //

}

}
