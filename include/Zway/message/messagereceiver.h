
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

#ifndef MESSAGE_RECEIVER_H_
#define MESSAGE_RECEIVER_H_

#include "Zway/packet.h"
#include "Zway/message/message.h"
#include "Zway/crypto/aes.h"
#include "Zway/crypto/digest.h"

namespace Zway {

// ============================================================ //

class Client;

class MessageReceiver
{
public:

    typedef std::shared_ptr<MessageReceiver> Pointer;

    static Pointer create(Client* client, UBJ::Value &head, UBJ::Value &contactPublicKey);

    bool process(PACKET pkt, const UBJ::Value &head);

    bool completed();

    MESSAGE message();

protected:

    MessageReceiver(Client* client, UBJ::Value &contactPublicKey);

    bool init(UBJ::Value &head);

protected:

    Client* m_client;

    BUFFER m_messageKey;

    UBJ::Value m_publicKey;

    int32_t m_messagePart;

    int32_t m_messageParts;

    int32_t m_partsProcessed;

    int32_t m_status;

    bool m_completed;

    MESSAGE m_msg;

    std::map<uint32_t, uint32_t> m_resourceParts;

    std::map<uint32_t, bool> m_skipResource;

    Crypto::AES m_aes;

    Crypto::Digest m_sha2;
};

typedef MessageReceiver::Pointer MESSAGE_RECEIVER;

typedef std::map<uint32_t, MESSAGE_RECEIVER> MESSAGE_RECEIVER_MAP;

// ============================================================ //

}

#endif /* MESSAGE_RECEIVER_H_ */
