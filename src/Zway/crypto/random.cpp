
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

#include "Zway/crypto/random.h"

#include <nettle/yarrow.h>
#include <stdio.h>

namespace Zway { namespace Crypto {

struct yarrow256_ctx yarrow;

// ============================================================ //

bool Random::setup()
{
    // init yarrow256

    yarrow256_init(&yarrow, 0, NULL);

    // seed yarrow256

    uint8_t seed[YARROW256_SEED_FILE_SIZE];

#if defined __gnu_linux__ || defined __MAC_10_6 || defined ANDROID

    FILE* pf = fopen("/dev/urandom", "rb");

    if (!pf) {

        return false;
    }

    fread(seed, sizeof(seed), 1, pf);

    fclose(pf);

#endif

    yarrow256_seed(&yarrow, sizeof(seed), seed);

    if (!yarrow256_is_seeded(&yarrow)) {

        return false;
    }

    return true;
}

// ============================================================ //

bool Random::random(uint8_t *data, uint32_t size, RandomLevel level)
{
    if (level == Weak) {

#if defined __gnu_linux__ || defined __MAC_10_6 || defined ANDROID

        FILE* pf = fopen("/dev/urandom", "rb");

        if (!pf) {

            return false;
        }

        fread(data, size, 1, pf);

        fclose(pf);
#endif

    }
    else
    if (level == Strong) {

#if defined __gnu_linux__ || defined __MAC_10_6 || defined ANDROID

        FILE* pf = fopen("/dev/urandom", "rb");

        if (!pf) {

            return false;
        }

        fread(data, size, 1, pf);

        fclose(pf);
#endif

    }
    else
    if (level == VeryStrong) {

        if (!yarrow256_is_seeded(&yarrow)) {

            return false;
        }

        yarrow256_random(&yarrow, size, data);
    }

    return true;
}

// ============================================================ //

void *Random::getYarrowCtx()
{
    return &yarrow;
}

// ============================================================ //

Random::Random()
{

}

// ============================================================ //

}

}
