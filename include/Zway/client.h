
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

#ifndef CLIENT_H_
#define CLIENT_H_

#include "Zway/event/eventdispatcher.h"
#include "Zway/request/dispatchrequest.h"
#include "Zway/request/createaccountrequest.h"
#include "Zway/request/loginrequest.h"
#include "Zway/request/configrequest.h"
#include "Zway/request/addcontactrequest.h"
#include "Zway/request/findcontactrequest.h"
#include "Zway/request/acceptcontactrequest.h"
#include "Zway/request/rejectcontactrequest.h"
#include "Zway/request/contactstatusrequest.h"
#include "Zway/request/inboxrequest.h"
#include "Zway/request/messagerequest.h"
#include "Zway/message/messagereceiver.h"
#include "Zway/message/messagesender.h"

#if defined _WIN32
#include <windows.h>
#endif

//#include <gnutls/x509.h>

namespace Zway {

// ============================================================ //

const uint32_t ZWAY_PORT = 5557;

const uint32_t HEARTBEAT_INTERVAL = 20000;
const uint32_t HEARTBEAT_TIMEOUT  = 20000;
const uint32_t RECONNECT_INTERVAL = 15000;

typedef std::map<uint32_t, uint32_t> CONTACT_STATUS_MAP;

class Client;

/**
* @brief The Sender class
*/

class Sender : public Thread
{
public:

    Sender(Client *client);

    bool busy();

    void notify();

    void cancel();

    void onRun();

protected:

    Client *m_client;

    ThreadSafe<bool> m_busy;

    std::mutex m_waitMutex;

    std::condition_variable m_waitCondition;

};

/**
* @brief The Client class
*/

class Client : public Thread
{
public:

    enum ClientStatus {

        Disconnected,

        Connecting,

        Connected,

        Secure,

        LoggedIn
    };

#if defined _WIN32

    static void initWSA();

    static void freeWSA();

#endif

    static bool startup(Client *client, const std::string &host, uint32_t port = ZWAY_PORT);

    static bool cleanup();


    static Client *instance() { return m_instance; }


    static void log(const char *msg, ...);


    virtual ~Client();

    void setEventHandler(EVENT_HANDLER handler);

    ClientStatus status();

    uint32_t lastHrtbSent();

    uint32_t lastHrtbRecv();

    static uint32_t tickCount();

    bool createAccount(
            const UBJ::Object &account,
            const std::string &storagePassword,
            CREATE_ACCOUNT_CALLBACK callback = nullptr);

    bool deleteAccount();

    bool login(
            STORAGE storage,
            LOGIN_CALLBACK callback = nullptr);

    bool setConfig(const UBJ::Value &config = UBJ::Object(), EVENT_CALLBACK callback = nullptr);

    bool addContact(
            const std::string &addCode,
            const std::string &label,
            const std::string &phone,
            ADD_CONTACT_CALLBACK callback = nullptr);

    bool createAddCode(ADD_CONTACT_CALLBACK callback = nullptr);

    bool findContact(const UBJ::Value &query, EVENT_CALLBACK callback = nullptr);

    bool acceptContact(uint32_t requestId, ACCEPT_CONTACT_CALLBACK callback = nullptr);

    bool rejectContact(uint32_t requestId, REJECT_CONTACT_CALLBACK callback = nullptr);

    bool requestContactStatus(const UBJ::Value &contacts = UBJ::Array());


    bool cancelRequest(uint32_t requestId, EVENT_CALLBACK callback = nullptr);


    void postEvent(EVENT event, bool immediately = false);

    bool postRequest(REQUEST request);

    bool postMessage(MESSAGE message);


    bool requestPending(Request::Type type);


    uint32_t getContactStatus(uint32_t id);


    void setStorageDir(const std::string& dir);

    std::string storageDir();

    STORAGE storage();

protected:

    void setStatus(ClientStatus status);

    void onRun();

    bool connect(const std::string& host, uint32_t port);

    void reconnect();

    void disconnect(bool bye = true, bool event = true);

    bool processContactRequest(const UBJ::Value &head);

    bool processContactRequestAccepted(const UBJ::Value &head);

    bool processContactRequestRejected(const UBJ::Value &head);

    bool processContactStatus(const UBJ::Value &head);

    bool processRequestPkt(PACKET pkt);

    bool processMessagePkt(PACKET pkt);

    void checkRequests(uint32_t *numIdle = nullptr, uint32_t *numWaiting = nullptr);

    bool processRequests();

    uint32_t processMessageSenders();

    uint32_t numMessageSenders();

    uint32_t sendPacket(PACKET pkt);

    uint32_t recvPacket(PACKET pkt);

    uint32_t send(uint8_t* data, uint32_t size);

    uint32_t recv(uint8_t* data, uint32_t size);

    bool readable(uint32_t ms);

    bool writable(uint32_t ms);

  //void parseCert();

    void setContactStatus(uint32_t contactId, uint32_t status);

protected:

    Client();

    virtual bool start(const std::string& host, uint32_t port = ZWAY_PORT);

    virtual bool close();

protected:

    static Client *m_instance;

    std::string m_host;

    uint32_t m_port;

#if defined _WIN32
    SOCKET m_socket;
#else
    int32_t m_socket;
#endif

    /*gnutls_session_t*/ void* m_session;

    /*gnutls_anon_client_credentials_t*/ void* m_anonCred;

    /*gnutls_certificate_credentials_t*/ void* m_certCred;


    Sender m_sender;

    EventDispatcher m_eventDispatcher;

    STORAGE m_storage;

    std::string m_storageDir;

    ThreadSafe<ClientStatus> m_status;

    ThreadSafe<uint32_t> m_lastHrtbSent;

    ThreadSafe<uint32_t> m_lastHrtbRecv;

    ThreadSafe<REQUEST_MAP> m_requests;

    ThreadSafe<MESSAGE_SENDER_LIST> m_messageSenders;

    ThreadSafe<MESSAGE_RECEIVER_MAP> m_messageReceivers;

    ThreadSafe<CONTACT_STATUS_MAP> m_contactStatus;

    // friends

    friend class Sender;

    friend class EventDispatcher;

    friend class Request;

    friend class LoginRequest;

    friend class CreateAccountRequest;

    friend class ConfigRequest;

    friend class FindContactRequest;

    friend class AddContactRequest;

    friend class AcceptContactRequest;

    friend class RejectContactRequest;

    friend class ContactStatusRequest;

    friend class InboxRequest;

    friend class MessageRequest;

    friend class MessageReceiver;

    friend class MessageSender;
};

// ============================================================ //

}

#endif /* CLIENT_H_ */
