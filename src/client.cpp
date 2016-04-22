
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

#include "Zway/client.h"

#if !defined _WIN32
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#endif

#include <cstdarg>
#include <cstdlib>
#include <gnutls/gnutls.h>

namespace Zway {

// ============================================================ //
// Client
// ============================================================ //

Client *Client::m_instance = nullptr;

#if defined _WIN32

void Client::initWSA()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
}

// ============================================================ //

void Client::freeWSA()
{
    WSACleanup();
}

#endif

// ============================================================ //

bool Client::startup(Client *client, const std::string &host, uint32_t port) {

    if (!m_instance && client) {

        m_instance = client;

        if (m_instance->start(host, port)) {

            return true;
        }
    }

    return false;
}

// ============================================================ //

bool Client::cleanup() {

    if (m_instance) {

        if (!m_instance->close()) {

        }

        delete m_instance;

        m_instance = nullptr;

        return true;
    }

    return false;
}

// ============================================================ //

void Client::log(const char *msg, ...)
{
    if (m_instance) {

        va_list args;
        va_start(args, msg);
        int len = vsnprintf(0, 0, msg, args);
        va_end(args);

        std::string str;

        if (len) {

            str.resize(len);

            va_start(args, msg);
            vsnprintf(&str[0], len + 1, msg, args);
            va_end(args);
        }

        m_instance->postEvent(Event::create(Event::Log, UBJ_OBJ("type" << 0 << "message" << str)));
    }
}

// ============================================================ //

Client::Client()
    : Thread(),
      m_port(0),
      m_socket(-1),
      m_session(nullptr),
      m_anonCred(nullptr),
      m_certCred(nullptr),
      m_sender(this),
      m_storage(nullptr),
      m_status(Disconnected),
      m_lastHrtbRecv(0),
      m_lastHrtbSent(0)
{

}

// ============================================================ //

Client::~Client()
{

}

// ============================================================ //

bool Client::start(const std::string& host, uint32_t port)
{
    // init gnutls

    int res = gnutls_global_init();

    if (res < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    res = gnutls_certificate_allocate_credentials((gnutls_certificate_credentials_t*)&m_certCred);

    if (res < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    res = gnutls_anon_allocate_client_credentials((gnutls_anon_client_credentials_t*)&m_anonCred);

    if (res < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    m_host = host;

    m_port = port;

    // run event dispatcher

    if (!m_eventDispatcher.run()) {

        //postEvent(MAKE_ERROR(0, "Failed to start event dispatcher"));

        return false;
    }

    // run client thread

    if (!run()) {

        //postEvent(MAKE_ERROR(0, "Failed to start client"));

        return false;
    }

    // run sender thread

    if (!m_sender.run()) {

        return false;
    }

    return true;
}

// ============================================================ //

bool Client::close()
{
    // cancel pending requests

    {
        MutexLocker locker(m_requests);

        m_requests->clear();
    }

    // cancel pending message senders

    {
        MutexLocker locker(m_messageSenders);

        m_messageSenders->clear();
    }

    // cancel pending message receivers

    {
        MutexLocker locker(m_messageReceivers);

        m_messageReceivers->clear();
    }

    // shutdown sender

    m_sender.cancelAndJoin();

    // shutdown client

    cancelAndJoin();

    disconnect();

    // shutdown event dispatcher

    m_eventDispatcher.cancelAndJoin();

    // close storage

    m_storage.reset();

    // release gnutls resources

    if (m_certCred) {

        gnutls_certificate_free_credentials((gnutls_certificate_credentials_t)m_certCred);
    }

    if (m_anonCred) {

        gnutls_anon_free_client_credentials((gnutls_anon_client_credentials_t)m_anonCred);
    }

    m_certCred = nullptr;

    m_anonCred = nullptr;

    gnutls_global_deinit();

    m_port = 0;

    return true;
}

// ============================================================ //

void Client::setEventHandler(EVENT_HANDLER handler)
{
    m_eventDispatcher.addHandler(handler);
}

// ============================================================ //

Client::ClientStatus Client::status()
{
    MutexLocker locker(m_status);

    return m_status;
}

// ============================================================ //

uint32_t Client::lastHrtbRecv()
{
    MutexLocker locker(m_lastHrtbRecv);

    return m_lastHrtbRecv;
}

// ============================================================ //

uint32_t Client::lastHrtbSent()
{
    MutexLocker locker(m_lastHrtbSent);

    return m_lastHrtbSent;
}

// ============================================================ //

uint32_t Client::tickCount()
{
    // TODO replace by std::chrono

#if defined _WIN32

    return GetTickCount();

#else

    struct timeval tv;

    gettimeofday(&tv, nullptr);

    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

#endif

    return 0;
}

// ============================================================ //

bool Client::createAccount(
        const UBJ::Object &account,
        const std::string &storagePassword,
        CREATE_ACCOUNT_CALLBACK callback)
{
    if (!m_storage &&
            status() >= Secure) {

        return postRequest(CreateAccountRequest::create(account, storagePassword, callback));
    }

    return false;
}

// ============================================================ //

bool Client::deleteAccount()
{
    return false;
}

// ============================================================ //

bool Client::login(
        STORAGE storage,
        LOGIN_CALLBACK callback)
{
    if (storage &&
            status() >= Secure &&
            status() != LoggedIn) {

        return postRequest(LoginRequest::create(storage, callback));
    }

    return false;
}

// ============================================================ //

bool Client::setConfig(const UBJ::Value &config, EVENT_CALLBACK callback)
{
    if (m_storage && m_storage->setConfig(config)) {

        if (status() >= LoggedIn) {

            // update server conf

            UBJ::Object conf;

            m_storage->getConfig(conf);

            // add contacts

            UBJ::Object contacts;

            Storage::NODE_LIST nodes = m_storage->getContacts();

            for (auto &node : nodes) {

                contacts[node->user1()] = UBJ_OBJ("notifyStatus" << 1);
            }

            conf["contacts"] = contacts;

            return postRequest(Zway::ConfigRequest::create(conf, callback));
        }
        else
            if (callback) {

                callback(DUMMY_EVENT(0));
            }
    }
    else
        if (callback) {

            callback(ERROR_EVENT(0, "Failed to set config"));
        }

    return false;
}

// ============================================================ //

bool Client::addContact(
        const std::string &addCode,
        const std::string &label,
        const std::string &phone,
        ADD_CONTACT_CALLBACK callback)
{
    if (status() < LoggedIn) {

        return false;
    }

    return postRequest(AddContactRequest::create(m_storage, addCode, label, phone, false, callback));
}

// ============================================================ //

bool Client::createAddCode(ADD_CONTACT_CALLBACK callback)
{
    if (status() < LoggedIn) {

        return false;
    }

    return postRequest(AddContactRequest::create(m_storage, std::string(), std::string(), std::string(), true, callback));
}

// ============================================================ //

bool Client::findContact(const UBJ::Value &query, EVENT_CALLBACK callback)
{
    if (status() < LoggedIn) {

        return false;
    }

    return postRequest(FindContactRequest::create(query, callback));
}

// ============================================================ //

bool Client::acceptContact(uint32_t requestId, ACCEPT_CONTACT_CALLBACK callback)
{
    if (status() < LoggedIn) {

        return false;
    }

    return postRequest(AcceptContactRequest::create(requestId, m_storage, callback));
}

// ============================================================ //

bool Client::rejectContact(uint32_t requestId, REJECT_CONTACT_CALLBACK callback)
{
    if (status() < LoggedIn) {

        return false;
    }

    return postRequest(RejectContactRequest::create(requestId, callback));
}

// ============================================================ //

bool Client::requestContactStatus(const UBJ::Value &contacts)
{
    if (status() < LoggedIn) {

        return false;
    }

    return postRequest(ContactStatusRequest::create(contacts, m_storage));
}

// ============================================================ //

bool Client::cancelRequest(uint32_t requestId, EVENT_CALLBACK callback)
{
    if (status() < LoggedIn) {

        return false;
    }

    UBJ::Object request;

    if (!m_storage->getRequest(requestId, request)) {

        return false;
    }

    postRequest(Zway::DispatchRequest::create(
                    UBJ_OBJ("requestDispatchId" << requestId << "action" << "cancel"),
                    callback));

    return true;
}

// ============================================================ //

void Client::postEvent(EVENT event, bool immediately)
{
    m_eventDispatcher.post(event);
}

// ============================================================ //

bool Client::postRequest(REQUEST request)
{
    MutexLocker locker(m_requests);

    request->setClient(this);

    request->start();

    (*m_requests)[request->id()] = request;

    return true;
}

// ============================================================ //

bool Client::postMessage(MESSAGE message)
{
    // set latest history id

    uint32_t historyId = m_storage->latestHistory(message->dst());

    message->setHistory(historyId);

    // create message sender

    MESSAGE_SENDER sender = MessageSender::create(this, message);

    if (!sender) {

        return false;
    }

    MutexLocker locker(m_messageSenders);

    m_messageSenders->push_back(sender);

    return true;
}

// ============================================================ //

bool Client::requestPending(Request::Type type)
{
    MutexLocker locker(m_requests);

    for (auto &it : *m_requests) {

        if (it.second->type() == type) {

            return true;
        }
    }

    return false;
}

// ============================================================ //

uint32_t Client::getContactStatus(uint32_t id)
{
    MutexLocker locker(m_contactStatus);

    if (m_contactStatus->find(id) != m_contactStatus->end()) {

        return (*m_contactStatus)[id];
    }

    return 0;
}

// ============================================================ //

void Client::setStatus(ClientStatus status)
{
    MutexLocker locker(m_status);

    m_status = status;
}

// ============================================================ //

void Client::onRun()
{
    // connect us to the server

    if (!connect(m_host, m_port)) {

        reconnect();

        //onThreadFinish();

        //return nullptr;
    }

    // loop

    for (;;) {

        // are we asked to cancel

        if (testCancel()) {

            break;
        }

        // check if the connection has been interrupted

        if (lastHrtbSent() > 0 && tickCount() >= lastHrtbSent() + HEARTBEAT_TIMEOUT) {

            postEvent(ERROR_EVENT(Event::ConnectionInterrupted, "Connection interrupted"));

            disconnect(false, false);

            reconnect();

            continue;
        }

        // receiver

        // wait for incoming packet

        PACKET pkt;

        if (readable(500)) {

            pkt = Packet::create();

            // read packet

            int32_t res = recvPacket(pkt);

            if (res <= 0) {

                // was the connection closed by the server

                if (res == 0) {

                }

                disconnect(false);

                reconnect();

                continue;
            }
            else {

                {
                    MutexLocker locker(m_lastHrtbSent);

                    m_lastHrtbSent = 0;
                }

                {
                    MutexLocker locker(m_lastHrtbRecv);

                    m_lastHrtbRecv = tickCount();
                }

                // process current packet

                switch (pkt->getId()) {

                case Packet::Heartbeat:

                    break;

                case Packet::Request:

                    processRequestPkt(pkt);

                    break;

                case Packet::Message:

                    processMessagePkt(pkt);

                    break;
                }
            }
        }

        // sender

        if (!m_sender.busy()) {

            // if there is work for the sender, wake it up

            uint32_t numIdle;

            checkRequests(&numIdle);

            if (numIdle || numMessageSenders() || (lastHrtbRecv() > 0 && tickCount() >= lastHrtbRecv() + HEARTBEAT_INTERVAL)) {

                m_sender.notify();
            }
        }
    }
}

// ============================================================ //

bool Client::connect(const std::string& host, uint32_t port)
{
    if (status() != Disconnected) {

        return false;
    }

    uint32_t a = inet_addr(host.c_str());

    if (a == INADDR_NONE) {

        struct hostent* he = gethostbyname(host.c_str());

        if (!he) {

            //postEvent(MAKE_ERROR(0, "Failed to resolve hostname"));

            return false;
        }

        a = *(uint32_t*)he->h_addr;
    }

#if defined _WIN32

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s == INVALID_SOCKET) {

        //postEvent(MAKE_ERROR(0, "Failed to create socket"));

        return false;
    }

    u_long on = 1;

    {
        int32_t ret = ioctlsocket(s, FIONBIO, &on);

        if (ret != NO_ERROR) {

            // ...
        }
    }

#else

    int32_t s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s == -1) {

        //postEvent(MAKE_ERROR(0, "Failed to create socket"));

        return false;
    }

    int32_t flags = fcntl(s, F_GETFL, 0);

    fcntl(s, F_SETFL, flags | O_NONBLOCK);

#endif

    // setup address and connect

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = a;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);

    setStatus(Connecting);

    int32_t res = ::connect(s, (struct sockaddr*)&addr, sizeof(addr));

#if defined _WIN32

    if (WSAGetLastError() == WSAEWOULDBLOCK) {

#else

    if (errno == EINPROGRESS) {

#endif
        // wait for socket to be connected

        uint32_t timeout = 10000;
        uint32_t ms = 0;

        while (ms < timeout) {

            if (testCancel()) {

                return false;
            }

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 200000;

            fd_set ws;
            FD_ZERO(&ws);
            FD_SET(s, &ws);

            if (select(s + 1, nullptr, &ws, nullptr, &tv) > 0) {

                res = -1;

#if defined _WIN32

                uint32_t len = sizeof(int32_t);

                getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&res, (int32_t*)&len);
#else

                socklen_t len = sizeof(int32_t);

                getsockopt(s, SOL_SOCKET, SO_ERROR, &res, &len);
#endif

                break;
            }

            ms += 200;
        }
    }

    if (res) {

#if defined _WIN32

        closesocket(s);

#else

        ::close(s);

#endif
        setStatus(Disconnected);

        postEvent(ERROR_EVENT(Event::ConnectionFailure, "Connection failed"));

        return false;
    }

    m_socket = s;

    setStatus(Connected);

    // init tls session

    if ((res = gnutls_init((gnutls_session_t*)&m_session, GNUTLS_CLIENT)) < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    // set priority

    if ((res = gnutls_priority_set_direct((gnutls_session_t)m_session, "NORMAL:+VERS-TLS1.2", nullptr)) < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    // set credentials

    if ((res = gnutls_credentials_set((gnutls_session_t)m_session, GNUTLS_CRD_ANON, m_anonCred)) < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    if ((res = gnutls_credentials_set((gnutls_session_t)m_session, GNUTLS_CRD_CERTIFICATE, m_certCred)) < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    // assign socket

    gnutls_transport_set_ptr((gnutls_session_t)m_session, (gnutls_transport_ptr_t)m_socket);

    // perform handshake

    do {

        res = gnutls_handshake((gnutls_session_t)m_session);
    }
    while (res < 0 && gnutls_error_is_fatal(res) == 0);

    if (res < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    // TODO: verify server certificate here

    //parseCert();

    setStatus(Secure);

    {
        MutexLocker locker(m_lastHrtbSent);

        m_lastHrtbSent = 0;
    }

    {
        MutexLocker locker(m_lastHrtbRecv);

        m_lastHrtbRecv = tickCount();
    }

    postEvent(Event::create(Event::ConnectionSuccess));

    return true;
}

// ============================================================ //

void Client::reconnect()
{
    for (;;) {

        uint32_t ms = 0;

        while (ms < RECONNECT_INTERVAL) {

            if (testCancel()) {

                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            ms += 200;
        }

        if (connect(m_host, m_port)) {

            postEvent(Event::create(Event::Reconnected));

            return;
        }
    }
}

// ============================================================ //

void Client::disconnect(bool bye, bool event)
{
    if (m_session) {

        if (bye) {

            int32_t res = gnutls_bye((gnutls_session_t)m_session, GNUTLS_SHUT_RDWR);

            if (gnutls_error_is_fatal(res)) {

                // ...
            }
        }

        gnutls_deinit((gnutls_session_t)m_session);

        m_session = nullptr;
    }

    if (m_socket) {

#if defined _WIN32

        closesocket(m_socket);

#else

        ::close(m_socket);

#endif

        m_socket = -1;

        setStatus(Disconnected);

        if (event) {

            postEvent(Event::create(Event::Disconnected));
        }
    }
}

// ============================================================ //

bool Client::processContactRequest(const UBJ::Value &head)
{
    uint32_t requestId = head["requestId"].toInt();

    UBJ::Object request;

    if (m_storage->getRequest(requestId, request)) {

        postEvent(Event::create(Event::ContactRequest, head));
    }
    else {

        // incoming contact request

        m_storage->addRequest(head);

        postEvent(Event::create(Event::ContactRequest, head));
    }

    return true;
}

// ============================================================ //

bool Client::processContactRequestAccepted(const UBJ::Value &head)
{
    uint32_t requestId = head["requestId"].toInt();

    uint32_t requestOrigId = head["requestOrigId"].toInt();

    UBJ::Object request;

    if (m_storage->getRequest(requestOrigId, request)) {

        // update request

        Storage::NODE node = m_storage->getNode(UBJ_OBJ("type" << Storage::Node::RequestType << "id" << requestOrigId));

        if (node) {

            node->setUser1(Request::Completed);
            node->setUser2(Request::AcceptContact);

            m_storage->updateNode(node, UBJ::Object(), false, false);
        }

        // add contact

        m_storage->addContact(
                UBJ_OBJ(
                    "contactId" << head["contactId"] <<
                    "label"     << head["label"] <<
                    "phone"     << head["phone"] <<
                    "publicKey" << head["publicKey"]));

        // set status

        setContactStatus(head["contactId"].toInt(), head["contactStatus"].toInt());

        // set config

        setConfig();

        // raise event

        postEvent(RequestEvent::create(Event::ContactRequestAccepted, nullptr, head));

        // dispatch request

        postRequest(DispatchRequest::create(UBJ_OBJ("requestDispatchId" << requestId)));

        return true;
    }

    return false;
}

// ============================================================ //

bool Client::processContactRequestRejected(const UBJ::Value &head)
{
    uint32_t requestId = head["requestId"].toInt();

    uint32_t requestOrigId = head["requestOrigId"].toInt();

    UBJ::Object request;

    if (m_storage->getRequest(requestOrigId, request)) {

        // update request

        Storage::NODE node = m_storage->getNode(UBJ_OBJ("type" << Storage::Node::RequestType << "id" << requestOrigId));

        if (node) {

            node->setUser1(Request::Completed);
            node->setUser2(Request::RejectContact);

            m_storage->updateNode(node, UBJ::Object(), false, false);
        }

        // raise event

        postEvent(RequestEvent::create(Event::ContactRequestRejected, nullptr, head));

        // dispatch request

        postRequest(DispatchRequest::create(UBJ_OBJ("requestDispatchId" << requestId)));


        return true;
    }

    return false;
}

// ============================================================ //

bool Client::processContactStatus(const UBJ::Value &head)
{
    // adjust contact status in our map

    MutexLocker locker(m_contactStatus);

    const UBJ::Value &contactStatus = head["contactStatus"];

    for (auto it = contactStatus.cbegin(); it != contactStatus.cend(); ++it) {

        const UBJ::Value &val = it->second;

        uint32_t contactId = atoi(it->first.c_str());

        (*m_contactStatus)[contactId] = val["status"].toInt();
    }

    // raise event

    postEvent(Event::create(
            Event::ContactStatus,
            contactStatus,
            UBJ::Object()));

    return true;
}

// ============================================================ //

bool Client::processRequestPkt(PACKET pkt)
{
    UBJ::Object head;

    if (!pkt->getHeadUbj(head)) {

        // TODO error event

        return false;
    }

    if (head.hasField("requestId")) {

        uint32_t requestId = head["requestId"].toInt();

        REQUEST request;

        {
            MutexLocker locker(m_requests);

            if (m_requests->find(requestId) != m_requests->end()) {

                request = (*m_requests)[requestId];
            }
        }

        // process request

        if (request) {

            if (!request->processRecv(pkt, head)) {

                // ...
            }
        }
    }

    if (head.hasField("requestType")) {

        uint32_t type = head["requestType"].toInt();

        if (type == Request::AddContact) {

            processContactRequest(head);
        }
        else
        if (type == Request::AcceptContact) {

            processContactRequestAccepted(head);
        }
        else
        if (type == Request::RejectContact) {

            processContactRequestRejected(head);
        }
        else
        if (type == Request::ContactStatus) {

            processContactStatus(head);
        }
    }

    return true;
}
// ============================================================ //

bool Client::processMessagePkt(PACKET pkt)
{
    UBJ::Object head;

    if (!pkt->getHeadUbj(head)) {

        return false;
    }

    uint32_t messageId  = head["messageId"].toInt();
    uint32_t messageSrc = head["messageSrc"].toInt();

    UBJ::Value publicKey;

    if (messageSrc != m_storage->accountId()) {

        // contact check

        UBJ::Object contact;

        if (!m_storage->getContact(messageSrc, contact)) {

            return false;
        }

        publicKey = contact["publicKey"];
    }
    else {

        publicKey = m_storage->publicKey();
    }

    // request access to message receivers

    MutexLocker locker(m_messageReceivers);

    // get receiver by message id

    MESSAGE_RECEIVER receiver = (*m_messageReceivers)[messageId];

    // if there is no receiver yet we create one

    if (!receiver) {

        receiver = MessageReceiver::create(this, head, publicKey);

        if (!receiver) {

            //postEvent(MAKE_ERROR(0, "Failed to create message receiver"));

            return false;
        }

        (*m_messageReceivers)[messageId] = receiver;
    }

    // let the receiver process the packet

    if (receiver->process(pkt, head)) {

        // check whether the receiver has completed

        if (receiver->completed()) {

            m_messageReceivers->erase(messageId);
        }
    }
    else {

        // TODO: handle situation please

        m_messageReceivers->erase(messageId);

        return false;
    }

    return true;
}

// ============================================================ //

void Client::checkRequests(uint32_t *numIdle, uint32_t *numWaiting)
{
    MutexLocker locker(m_requests);

    std::list<REQUEST> completed;

    uint32_t nIdle = 0;
    uint32_t nWaiting = 0;

    for (auto &it : *m_requests) {

        REQUEST &req = it.second;

        if (req->status() == Request::Completed ||
                req->status() == Request::Timeout ||
                req->status() == Request::Error) {

            completed.push_back(req);
        }

        if (req->status() != Request::Sending && req->checkTimeout()) {

            completed.push_back(req);

            postEvent(RequestEvent::create(
                    Event::RequestTimeout,
                    req,
                    UBJ::Object(),
                    UBJ::Object(),
                    [req] (EVENT event) {
                        req->invokeCallback(event);
                    }));
        }
        else
            if (req->status() == Request::Idle) {

                nIdle++;
            }
            else
                if (req->status() == Request::WaitingForResponse) {

                    nWaiting++;
                }
    }

    // remove completed requests

    for (auto &req : completed) {

        m_requests->erase(req->id());
    }

    if (numIdle) {

        *numIdle = nIdle;
    }

    if (numWaiting) {

        *numWaiting = nWaiting;
    }
}

// ============================================================ //

bool Client::processRequests()
{
    REQUEST_MAP requests;

    // get snapshot of current requests to enable requests to spawn requests

    {
        MutexLocker locker(m_requests);

        requests = *m_requests;
    }

    // process requests

    for (auto &it : requests) {

        auto &request = it.second;

        request->processSend();
    }

    return true;
}

// ============================================================ //

uint32_t Client::processMessageSenders()
{
    uint32_t i=0;

    MutexLocker locker(m_messageSenders);

    for (auto it = m_messageSenders->begin(); it != m_messageSenders->end(); ++it, i++) {

        MESSAGE_SENDER sender = *it;

        if (sender->process()) {

            if (sender->completed()) {

                m_messageSenders->erase(it++);
            }
        }
        else {

            m_messageSenders->erase(it);
        }
    }

    return i;
}

// ============================================================ //

uint32_t Client::numMessageSenders()
{
    MutexLocker locker(m_messageSenders);

    return m_messageSenders->size();
}

// ============================================================ //

uint32_t Client::sendPacket(PACKET pkt)
{
    uint32_t s = 0;

    uint32_t r = send((uint8_t*)&pkt->getId(), PACKET_BASE_SIZE);

    if (r < PACKET_BASE_SIZE) {

        return -1;
    }

    s += r;

    if (pkt->getHeadSize() > 0) {

        r = send(pkt->getHead()->data(), pkt->getHeadSize());

        if (r < pkt->getHeadSize()) {

            return -1;
        }

        s += r;
    }

    if (pkt->getBodySize() > 0) {

        r = send(pkt->getBody()->data(), pkt->getBodySize());

        if (r < pkt->getBodySize()) {

            return -1;
        }

        s += r;
    }

    return s;
}

// ============================================================ //

uint32_t Client::recvPacket(PACKET pkt)
{
    uint32_t s = 0;

    uint32_t r = recv((uint8_t*)&pkt->getId(), PACKET_BASE_SIZE);

    if (r < PACKET_BASE_SIZE) {

        return r;
    }

    s += r;

    if (pkt->getHeadSize() > 0) {

        if (pkt->getHeadSize() > MAX_PACKET_HEAD) {

            return -1;
        }

        pkt->setHead(Buffer::create(nullptr, pkt->getHeadSize()));

        r = recv(pkt->getHead()->data(), pkt->getHeadSize());

        if (r < pkt->getHeadSize()) {

            return r;
        }

        s += r;
    }

    if (pkt->getBodySize() > 0) {

        if (pkt->getBodySize() > MAX_PACKET_BODY) {

            return -1;
        }

        pkt->setBody(Buffer::create(nullptr, pkt->getBodySize()));

        r = recv(pkt->getBody()->data(), pkt->getBodySize());

        if (r < pkt->getBodySize()) {

            return -1;
        }

        s += r;
    }

    return s;
}

// ============================================================ //

uint32_t Client::send(uint8_t* data, uint32_t size)
{
    uint32_t s = 0;

    while (s < size) {

        if (m_sender.testCancel()) {

            break;
        }

        if (!writable(200)) {

            continue;
        }

        int32_t ret;

        ret = gnutls_record_send((gnutls_session_t)m_session, &data[s], size - s);

        if (ret == GNUTLS_E_AGAIN) {

        }

        if (gnutls_error_is_fatal(ret)) {

            return -1;
        }

        if (ret > 0) {

            s += ret;
        }
    }

    return s;
}

// ============================================================ //

uint32_t Client::recv(uint8_t* data, uint32_t size)
{
    uint32_t s = 0;

    while (s < size) {

        if (testCancel()) {

            break;
        }

        if (!readable(200)) {

            continue;
        }

        int32_t ret;

        ret = gnutls_record_recv((gnutls_session_t)m_session, &data[s], size - s);

        if (gnutls_error_is_fatal(ret)) {

            return -1;
        }

        if (ret > 0) {

            s += ret;
        }
    }

    return s;
}

// ============================================================ //

bool Client::readable(uint32_t ms)
{
    // check if there is data left over from previous read

    if (gnutls_record_check_pending((gnutls_session_t)m_session)) {

        return true;
    }

    // check for socket read readiness

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(m_socket, &fds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = ms * 1000;

    int32_t res = select(m_socket+1, &fds, nullptr, nullptr, &tv);

    if (res <= 0) {

        return false;
    }

    return true;
}

// ============================================================ //

bool Client::writable(uint32_t ms)
{
    // check for socket write readiness

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(m_socket, &fds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = ms * 1000;

    int32_t res = select(m_socket+1, nullptr, &fds, nullptr, &tv);

    if (res <= 0) {

        return false;
    }

    return true;
}

// ============================================================ //

void Client::setContactStatus(uint32_t contactId, uint32_t status)
{
    MutexLocker locker(m_contactStatus);

    (*m_contactStatus)[contactId] = status;
}

// ============================================================ //

void Client::setStorageDir(const std::string &dir)
{
    m_storageDir = dir;
}

// ============================================================ //

std::string Client::storageDir()
{
    return m_storageDir;
}

// ============================================================ //

STORAGE Client::storage()
{
    return m_storage;
}

// ============================================================ //
// Sender
// ============================================================ //

Sender::Sender(Client *client)
    : Thread(),
      m_client(client),
      m_busy(false)
{

}

// ============================================================ //

bool Sender::busy()
{
    MutexLocker locker(m_busy);

    return m_busy;
}

// ============================================================ //

void Sender::notify()
{
    m_waitCondition.notify_one();
}

// ============================================================ //

void Sender::cancel()
{
    Thread::cancel();

    m_waitCondition.notify_one();
}

// ============================================================ //

void Sender::onRun()
{
    for (;;) {

        if (testCancel()) {

            break;
        }

        if (m_client->status() < Client::Secure) {

            std::this_thread::sleep_for(std::chrono::seconds(1));

            continue;
        }

        uint32_t numIdle;

        // check for work

        m_client->checkRequests(&numIdle);

        if (!numIdle || !m_client->numMessageSenders()) {

            // wait for work to do

            {
                MutexLocker locker(m_busy);

                m_busy = false;
            }

            std::unique_lock<std::mutex> locker(m_waitMutex);

            m_waitCondition.wait(locker);
        }

        // check for work again

        m_client->checkRequests(&numIdle);

        if (numIdle || m_client->numMessageSenders()) {

            {
                MutexLocker locker(m_busy);

                m_busy = true;
            }

            // process requests

            m_client->processRequests();

            // process messages

            m_client->processMessageSenders();
        }
        else
        if (m_client->lastHrtbRecv() > 0 && m_client->tickCount() >= m_client->lastHrtbRecv() + HEARTBEAT_INTERVAL) {

            // send heartbeat

            {
                MutexLocker locker(m_client->m_lastHrtbSent);

                m_client->sendPacket(Packet::create(Packet::Heartbeat));

                m_client->m_lastHrtbSent = m_client->tickCount();
            }

            {
                MutexLocker locker(m_client->m_lastHrtbRecv);

                m_client->m_lastHrtbRecv = 0;
            }
        }
    }
}

// ============================================================ //

/*
void Client::parseCert()
{
    char serial[40];
    char dn[256];
    size_t size;
    unsigned int algo, bits;
    time_t expiration_time, activation_time;
    const gnutls_datum_t* cert_list;
    unsigned int cert_list_size = 0;
    gnutls_x509_crt_t cert;
    gnutls_datum_t cinfo;

    if (gnutls_certificate_type_get(m_session) != GNUTLS_CRT_X509) {

        return;
    }

    cert_list = gnutls_certificate_get_peers(m_session, &cert_list_size);

    std::cout << "Peer provided " << cert_list_size << " certificate(s)\n";

    if (cert_list_size > 0) {

        int ret;

        gnutls_x509_crt_init(&cert);

        gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER);

        std::cout << "Certificate info:\n";

        ret = gnutls_x509_crt_print(cert, GNUTLS_CRT_PRINT_ONELINE, &cinfo);
        if (ret == 0) {

            std::cout << cinfo.data << "\n";

            gnutls_free(cinfo.data);
        }

        gnutls_x509_crt_deinit(cert);
    }
}
*/

// ============================================================ //

}
