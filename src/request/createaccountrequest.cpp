
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

#include "Zway/request/createaccountrequest.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

CREATE_ACCOUNT_REQUEST CreateAccountRequest::create(
        const UBJ::Object &account,
        const std::string& storagePassword,
        Callback callback)
{
    return CREATE_ACCOUNT_REQUEST(new CreateAccountRequest(account, storagePassword, callback));
}

// ============================================================ //

CreateAccountRequest::CreateAccountRequest(
        const UBJ::Object &account,
        const std::string& storagePassword,
        Callback callback)
    : Request(CreateAccount, DEFAULT_TIMEOUT, 0),
      m_storagePassword(storagePassword),
      m_callback(callback)
{
    // create rsa key pair

    UBJ::Object publicKey;
    UBJ::Object privateKey;

    if (Crypto::RSA::createKeyPair(publicKey, privateKey, 2048)) {

        m_keys["k1"] = publicKey;
        m_keys["k2"] = privateKey;
    }
    else {

        setStatus(Error);

        return;

    }

    // set request args

    m_head["label"] = account["label"];
    m_head["findByLabel"] = account["findByLabel"];
    m_head["findByPhone"] = account["findByPhone"];
}

// ============================================================ //

bool CreateAccountRequest::processRecv(PACKET /*pkt*/, const UBJ::Value &head)
{
    uint32_t status = head["status"].toInt();

    if (status == 1) {

        finish();

        UBJ::Object account;

        account["label"] = m_head["label"];

        account["id"] = head["accountId"];

        account["pw"] = head["accountPw"];

        account["k1"] = m_keys["k1"];

        account["k2"] = m_keys["k2"];

        m_storageFilename = m_client->storageDir() + Crypto::Digest::digestHexStr(account["label"].buffer()) + ".store";

        m_storage = Storage::init(m_storageFilename, m_storagePassword, account);

        if (m_storage) {

            // set initial config

            m_storage->setConfig(UBJ_OBJ(
                    "findByLabel"  << m_head["findByLabel"] <<
                    "findByPhone"  << m_head["findByPhone"] <<
                    "notifyStatus" << 1));

            m_client->postEvent(RequestEvent::create(
                    0,
                    shared_from_this(),
                    UBJ::Object(),
                    UBJ::Object(),
                    [this] (EVENT event) {
                        invokeCallback(event);
                    }));
        }
        else {

            m_client->postEvent(RequestEvent::create(
                    0,
                    shared_from_this(),
                    UBJ::Object(),
                    ERROR_INFO("failed to create storage"),
                    [this] (EVENT event) {
                        invokeCallback(event);
                    }));
        }
	}
	else
    if (status == 0) {

        finish();

        m_client->postEvent(RequestEvent::create(
                0,
                shared_from_this(),
                UBJ::Object(),
                ERROR_INFO(head["message"]),
                [this] (EVENT event) {
                    invokeCallback(event);
                }));
    }

    return true;
}

// ============================================================ //

void CreateAccountRequest::invokeCallback(EVENT event)
{
    if (m_callback) {
        m_callback(
            RequestEvent::cast(event),
            std::dynamic_pointer_cast<CreateAccountRequest>(shared_from_this()));
    }
}

// ============================================================ //

STORAGE CreateAccountRequest::storage()
{
    return m_storage;
}

// ============================================================ //

std::string CreateAccountRequest::storageFilename()
{
    return m_storageFilename;
}

// ============================================================ //

}
