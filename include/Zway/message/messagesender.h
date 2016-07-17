
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

#ifndef MESSAGE_SENDER_H_
#define MESSAGE_SENDER_H_

#include "Zway/message/message.h"
#include "Zway/crypto/aes.h"
#include "Zway/crypto/digest.h"

#include <list>

namespace Zway {

// ============================================================ //

class Client;

class MessageSender
{
public:

    typedef std::shared_ptr<MessageSender> Pointer;

    static Pointer create(Client *client, MESSAGE msg);

    bool init();

    bool process();

    bool completed();

protected:

    MessageSender(Client *client, MESSAGE msg);

    void incrementSalt();

protected:

    Client* m_client;

    uint32_t m_messageSize;

    uint32_t m_messagePart;

    uint32_t m_messageParts;

    uint32_t m_resourcePart;

    uint32_t m_resourceIndex;

    uint32_t m_status;

    bool m_completed;

    MESSAGE m_msg;

    RESOURCE m_res;

    RESOURCE_LIST m_resList;

    std::map<uint32_t, uint32_t> m_resourceParts;

    std::map<uint32_t, bool> m_noStoreResource;

    UBJ::Object m_meta;

    BUFFER m_messageKey;

    std::map<uint32_t, BUFFER> m_messageKeysEnc;

    BUFFER m_salt;

    Crypto::AES m_aes;

    Crypto::Digest m_sha2;
};

typedef MessageSender::Pointer MESSAGE_SENDER;

typedef std::list<MESSAGE_SENDER> MESSAGE_SENDER_LIST;

// ============================================================ //

}

#endif /* MESSAGE_SENDER_H_ */
