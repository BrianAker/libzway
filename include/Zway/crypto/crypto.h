
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

#ifndef CRYPTO_H_
#define CRYPTO_H_

#include "Zway/crypto/random.h"
#include "Zway/crypto/digest.h"
#include "Zway/crypto/aes.h"
#include "Zway/crypto/rsa.h"

#include <string>

namespace Zway { namespace Crypto {

// ============================================================ //

bool setup();

void cleanup();

uint32_t mkId();

std::string hexStr(uint8_t* data, uint32_t size);

std::string hexStr(BUFFER data);

// ============================================================ //

}

}

#endif /* CRYPTO_H_ */
