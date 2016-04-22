
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

#include "Zway/storage/storage.h"
#include "Zway/client.h"
#include <sqlite3.h>
#include <cstring>
#include <fstream>
#include <sstream>

namespace Zway {

const uint32_t STORAGE_VERSION = 1;

// ============================================================ //
// Storage
// ============================================================ //

Storage::Pointer Storage::init(const std::string &filename, const std::string &password, UBJ::Value &info)
{
    STORAGE storage = STORAGE(new Storage());

    if (storage->_init(filename, password, info)) {

        return storage;
    }

    return nullptr;
}

// ============================================================ //

Storage::Pointer Storage::open(const std::string &filename, const std::string &password)
{
    STORAGE storage = STORAGE(new Storage());

    if (storage->_open(filename, password)) {

        return storage;
    }

    return nullptr;
}

// ============================================================ //

Storage::Storage()
    : m_db(NULL),
      m_accountId(0)
{

}

// ============================================================ //

Storage::~Storage()
{
    close();
}

// ============================================================ //

bool Storage::_init(const std::string &filename, const std::string &password, UBJ::Value &info)
{
    FILE* pf = fopen(filename.c_str(), "r");
    if (pf) {

        fclose(pf);

        return false;
    }

    if (sqlite3_open_v2(
            filename.c_str(),
            (sqlite3**)&m_db,
            SQLITE_OPEN_CREATE |
            SQLITE_OPEN_READWRITE |
            SQLITE_OPEN_FULLMUTEX,
            nullptr) != SQLITE_OK) {

        return false;
    }

    std::string sql;
    char* errmsg;

    // create table "nodes"

    sql = "CREATE TABLE nodes ("\
          "id     INTEGER,"\
          "time   INTEGER,"\
          "type   INTEGER,"\
          "parent INTEGER,"\
          "name   TEXT,"\
          "user1  INTEGER,"\
          "user2  INTEGER,"\
          "user3  TEXT,"\
          "user4  TEXT,"\
          "head   BLOB,"
          "body   BLOB"
          ")";

    sqlite3_exec((sqlite3*)m_db, sql.c_str(), nullptr, nullptr, &errmsg);

    if (errmsg) {

        sqlite3_free(errmsg);

        close();

        return false;
    }

    // create random storage key

    m_key = Buffer::create(nullptr, 32);

    if (!Crypto::Random::random(m_key->data(), m_key->size(), Crypto::Random::VeryStrong)) {

        return false;
    }

    // create sha2 digest from password

    BUFFER pwd = Crypto::Digest::digest((uint8_t*)password.c_str(), password.length(), Crypto::Digest::DIGEST_SHA256);

    // clear password

    memset((void*)&password[0], 0, password.size());

    // encrypt storage key with the hashed password

    Crypto::AES aes;

    BUFFER key = Buffer::create(nullptr, 32);
    BUFFER ctr = Buffer::create(nullptr, 16);

    aes.setKey(pwd);
    aes.setCtr(ctr);
    aes.encrypt(m_key, key, 32);

    // encrypt storage password

    aes.setCtr(ctr);
    aes.encrypt(pwd, pwd, 32);

    // create root node;

    NODE root = Node::create();

    root->setId(RootNodeId);

    root->setUser1(STORAGE_VERSION);

    UBJ::Object head;

    // store encrypted key

    head["key"] = key;

    // store encrypted password

    head["pwd"] = pwd;

    // set head

    if (!root->setHeadUbj(head)) {

        return false;
    }

    if (!addNode(root, false)) {

        return false;
    }

    // create data node

    NODE data = Node::create();

    data->setId(DataNodeId);

    data->setParent(RootNodeId);

    if (!data->setBodyUbj(info)) {

        return false;
    }

    if (!addNode(data)) {

        return false;
    }

    m_accountId = info["id"].toInt();

    m_accountPw = info["pw"].toInt();

    m_accountLabel = info["label"].toString();

    m_privateKey = info["k1"];

    m_publicKey = info["k2"];

    createDefaultNodes();

    return true;
}

// ============================================================ //

bool Storage::_open(const std::string &filename, const std::string &password)
{
    if (sqlite3_open_v2(
            filename.c_str(),
            (sqlite3**)&m_db,
            SQLITE_OPEN_READWRITE |
            SQLITE_OPEN_FULLMUTEX,
            nullptr) != SQLITE_OK) {

        return false;
    }

    NODE root = getNode(UBJ_OBJ("id" << RootNodeId), UBJ::Object(), UBJ::Object(), 0, false);

    if (!root) {

        close();

        return false;
    }

    UBJ::Object head;

    if (!root->headUbj(head)) {

        close();

        return false;
    }

    if (!head.hasField("key") || !head["key"].size() ||
        !head.hasField("pwd") || !head["pwd"].size()) {

        close();

        return false;
    }

    // create sha2 digest from password

    BUFFER digest = Crypto::Digest::digest((uint8_t*)password.c_str(), password.size(), Crypto::Digest::DIGEST_SHA256);

    // clear password

    memset((void*)&password[0], 0, password.size());

    // decrypt storage key

    Crypto::AES aes;

    BUFFER key = Buffer::create(head["key"].buffer());
    BUFFER ctr = Buffer::create(nullptr, 16);

    aes.setKey(digest);
    aes.setCtr(ctr);
    aes.decrypt(key, key, 32);

    // decrypt storage password

    BUFFER pwd = Buffer::create(head["pwd"].buffer());

    aes.setCtr(ctr);
    aes.decrypt(pwd, pwd, 32);

    // verify password

    if (memcmp(pwd->data(), digest->data(), 32)) {

        // password mismatch

        close();

        return false;
    }

    // assign decrypted key

    m_key = key;

    // load data

    NODE data = getNode(UBJ_OBJ("id" << DataNodeId), UBJ::Object(), UBJ::Object(), 0, true, true);

    UBJ::Object body;

    if (!data) {

        close();

        return false;
    }

    if (!data->bodyUbj(body, true)) {

        close();

        return false;
    }

    m_accountId = body["id"].toInt();

    m_accountPw = body["pw"].toInt();

    m_accountLabel = body["label"].toString();

    m_privateKey = body["k1"];

    m_publicKey = body["k2"];

    createDefaultNodes();

    return true;
}

// ============================================================ //

bool Storage::createDefaultNodes()
{
    if (!getNodeCount(UBJ_OBJ("id" << VfsNodeId << "parent" << RootNodeId))) {

        // create vfs node

        NODE node = Node::create(Node::DirectoryType, RootNodeId);

        node->setId(VfsNodeId);

        if (!addNode(node)) {

            return false;
        }
    }

    if (!getNodeCount(UBJ_OBJ("id" << ConfigNodeId << "parent" << RootNodeId))) {

        // create vfs node

        NODE node = Node::create(Node::UnknownType, RootNodeId);

        node->setId(ConfigNodeId);

        if (!addNode(node)) {

            return false;
        }
    }

    return true;
}

// ============================================================ //

void Storage::close()
{
    if (m_db) {

        sqlite3_close((sqlite3*)m_db);
    }

    m_db = nullptr;

    m_key.reset();

    m_accountId = 0;

    m_openBlobs.clear();

    m_openBlobsAes.clear();

    m_contactLabels.clear();
}

// ============================================================ //

uint32_t Storage::accountId()
{
    return m_accountId;
}


// ============================================================ //

uint32_t Storage::accountPw()
{
    return m_accountPw;
}

// ============================================================ //

std::string Storage::accountLabel()
{
    return m_accountLabel;
}

// ============================================================ //

UBJ::Object &Storage::privateKey()
{
    return m_privateKey;
}

// ============================================================ //

UBJ::Object &Storage::publicKey()
{
    return m_publicKey;
}

// ============================================================ //

bool Storage::addNode(std::shared_ptr<Node> node, bool encrypt)
{
    if (!node) {

        return false;
    }

    // check for existing node with same id

    if (getNode(UBJ_OBJ("id" << node->id()), UBJ::Object(), UBJ::Array(), 0, encrypt)) {

        return false;
    }

    // insert node

    UBJ::Object insert;
    insert["id"]     = node->id();
    insert["time"]   = node->time();
    insert["type"]   = node->type();
    insert["parent"] = node->parent();
    insert["name"]   = node->name();
    insert["user1"]  = node->user1();
    insert["user2"]  = node->user2();
    insert["user3"]  = node->user3();
    insert["user4"]  = node->user4();

    if (node->head()) {

        insert["head"] = node->head();
    }

    if (node->body()) {

        insert["body"] = node->body();
    }

    Action action = prepareInsert("nodes", insert, encrypt);

    sqlite3_stmt* stmt = (sqlite3_stmt*)action.stmt();

    if (!stmt) {

        return false;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {

        sqlite3_finalize(stmt);

        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}

// ============================================================ //

Storage::NODE Storage::getNode(
        const UBJ::Object &query,
        const UBJ::Object &order,
        const UBJ::Array &fieldsToReturn,
        int32_t offset,
        bool decrypt,
        bool secure)
{
    NODE_LIST res = getNodes(query, order, fieldsToReturn, 1, offset, decrypt, secure);

    if (!res.empty()) {

        return res.front();
    }

    return nullptr;
}

// ============================================================ //

Storage::NODE_LIST Storage::getNodes(
        const UBJ::Object &query,
        const UBJ::Object &order,
        const UBJ::Array &fieldsToReturn,
        int32_t limit,
        int32_t offset,
        bool decrypt,
        bool secure)
{
    NODE_LIST res;

    Action action = prepareSelect("nodes", query, order, fieldsToReturn, limit, offset, decrypt);

    sqlite3_stmt* stmt = (sqlite3_stmt*)action.stmt();

    if (!stmt) {

        return res;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        UBJ::Object meta;

        rowToUbj(stmt, meta, decrypt);

        NODE node = makeNode(meta, stmt, decrypt, secure);

        res.push_back(node);
    }

    sqlite3_finalize(stmt);

    return res;
}

// ============================================================ //

bool Storage::updateNode(uint32_t nodeId, const UBJ::Object &update, bool encrypt)
{
    UBJ::Value where = UBJ_OBJ("id" << nodeId);

    Action action = prepareUpdate("nodes", update, where, encrypt);

    sqlite3_stmt* stmt = (sqlite3_stmt*)action.stmt();

    if (!stmt) {

        return false;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {

        sqlite3_finalize(stmt);

        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}

// ============================================================ //

bool Storage::updateNode(
        NODE node,
        const UBJ::Array &fieldsToUpdate,
        bool updateHead,
        bool updateBody,
        bool encrypt)
{
    if (!node) {

        return false;
    }

    UBJ::Value update;

    if (fieldsToUpdate.numValues()) {

        for (auto it = fieldsToUpdate.cbegin(); it != fieldsToUpdate.cend(); ++it) {

            std::string key = it->first;

            // ...
        }
    }
    else {

        update = UBJ_OBJ(
                "id"     << node->id() <<
                "time"   << node->time() <<
                "type"   << node->type() <<
                "parent" << node->parent() <<
                "name"   << node->name() <<
                "user1"  << node->user1() <<
                "user2"  << node->user2() <<
                "user3"  << node->user3() <<
                "user4"  << node->user4());
    }

    if (updateHead) {

        update["head"] = node->head();
    }

    if (updateBody) {

        update["body"] = node->body();
    }

    if (!update.numValues()) {

        return false;
    }

    if (!updateNode(node->id(), update, encrypt)) {

        return false;
    }

    return true;
}

// ============================================================ //

bool Storage::updateNodeHead(NODE node, bool encrypt)
{
    if (node) {

        return updateNode(node->id(), UBJ_OBJ("head" << node->head()), encrypt);
    }

    return false;
}

// ============================================================ //

bool Storage::updateNodeBody(NODE node, bool encrypt)
{
    if (node) {

        return updateNode(node->id(), UBJ_OBJ("body" << node->body()), encrypt);
    }

    return false;
}

// ============================================================ //

bool Storage::deleteNode(const UBJ::Object &query, bool deleteChildren, bool encryptQuery)
{
    NODE node = getNode(query, UBJ::Object(), UBJ_ARR("id"), 0, encryptQuery);

    if (!node) {

        return false;
    }

    if (node->id() == RootNodeId ||
            node->id() == DataNodeId ||
            node->id() == VfsNodeId ||
            node->id() == ConfigNodeId) {

        return false;
    }

    if (deleteChildren) {

        NODE_LIST childNodes = getNodes(
                UBJ_OBJ("parent" << node->id()),
                UBJ::Object(),
                UBJ_ARR("id"),
                0,
                0,
                encryptQuery);

        for (auto &childNode : childNodes) {

            if (!deleteNode(UBJ_OBJ("id" << childNode->id()))) {

                return false;
            }
        }
    }

    // zero node body

    zeroBodyBlob(node->id());

    // delete node

    Action action = prepareDelete("nodes", UBJ_OBJ("id" << node->id()), encryptQuery);

    sqlite3_stmt* stmt = (sqlite3_stmt*)action.stmt();

    if (sqlite3_step(stmt) != SQLITE_DONE) {

        sqlite3_finalize(stmt);

        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}

// ============================================================ //

bool Storage::deleteNodes(const UBJ::Object &query, bool deleteChildren, bool encryptQuery)
{
    NODE_LIST nodes = getNodes(query, UBJ::Object(), UBJ_ARR("id" << "user1"), 0, 0, encryptQuery);

    for (auto &node : nodes) {

        if (!deleteNode(UBJ_OBJ("id" << node->id()), deleteChildren, encryptQuery)) {

            return false;
        }
    }

    return true;
}

// ============================================================ //

uint32_t Storage::getNodeCount(const UBJ::Object &query, bool encrypt)
{
    Action action = prepareCount("nodes", query, encrypt);

    sqlite3_stmt* stmt = (sqlite3_stmt*)action.stmt();

    if (!stmt) {

        return false;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {

        sqlite3_finalize(stmt);

        return false;
    }

    uint32_t res = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return res;
}

// ============================================================ //

bool Storage::openBodyBlob(uint32_t id, bool encryptQuery)
{
    if (m_openBlobs.find(id) != m_openBlobs.end()) {

        return false;
    }

    Action action = prepareSelect(
            "nodes",
            UBJ_OBJ("id" << id),
            UBJ::Object(),
            UBJ_ARR("rowid"),
            0,
            0,
            encryptQuery);

    sqlite3_stmt* stmt = (sqlite3_stmt*)action.stmt();

    if (!stmt) {

        return false;
    }

    int rowId = -1;

    if (sqlite3_step(stmt) == SQLITE_ROW) {

        rowId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    if (rowId > 0) {

        sqlite3_blob* blob;

        if (sqlite3_blob_open((sqlite3*)m_db, "main", "nodes", "body", rowId, 1, &blob) != SQLITE_OK) {

            return false;
        }
        else {

            m_openBlobs[id] = blob;

            m_openBlobsAes[id] = new Crypto::AES();

            m_openBlobsAes[id]->setKey(m_key);

            m_openBlobsAes[id]->setCtr(Buffer::create(nullptr, 16));

            return true;
        }
    }

    return false;
}

// ============================================================ //

bool Storage::closeBodyBlob(uint32_t id)
{
    if (m_openBlobs.find(id) == m_openBlobs.end()) {

        return false;
    }

    sqlite3_blob* blob = (sqlite3_blob*)m_openBlobs[id];

    sqlite3_blob_close(blob);

    m_openBlobs.erase(id);

    delete m_openBlobsAes[id];

    m_openBlobsAes.erase(id);

    return true;
}

// ============================================================ //

bool Storage::readBodyBlob(uint32_t id, uint8_t *data, uint32_t size, uint32_t offset)
{
    if (m_openBlobs.find(id) == m_openBlobs.end()) {

        return false;
    }

    sqlite3_blob* blob = (sqlite3_blob*)m_openBlobs[id];

    if (sqlite3_blob_read(blob, data, size, offset) != SQLITE_OK) {

        return false;
    }

    // ...

    return true;
}

// ============================================================ //

bool Storage::writeBodyBlob(uint32_t id, uint8_t *data, uint32_t size, uint32_t offset)
{
    if (m_openBlobs.find(id) == m_openBlobs.end()) {

        return false;
    }

    BUFFER buf = Buffer::create(data, size);

    sqlite3_blob* blob = (sqlite3_blob*)m_openBlobs[id];

    m_openBlobsAes[id]->encrypt(buf, buf, buf->size());

    if (sqlite3_blob_write(blob, buf->data(), buf->size(), offset) != SQLITE_OK) {

        return false;
    }

    return true;
}

// ============================================================ //

bool Storage::zeroBodyBlob(uint32_t id)
{
    if (!openBodyBlob(id)) {

        return false;
    }

    uint32_t size = openBlobsSize(id);

    if (!size) {

        closeBodyBlob(id);

        return false;
    }

    uint8_t zero[1024];
    memset(zero, 0, sizeof(zero));

    uint32_t i=0;

    while (i < size) {

        uint32_t bytesToWrite = sizeof(zero);

        if (size - i < bytesToWrite) {

            bytesToWrite = size - i;
        }

        writeBodyBlob(id, zero, bytesToWrite, i);

        i += bytesToWrite;
    }

    closeBodyBlob(id);

    return true;
}

// ============================================================ //

uint32_t Storage::openBlobsSize(uint32_t id)
{
    if (m_openBlobs.find(id) != m_openBlobs.end()) {

        return sqlite3_blob_bytes((sqlite3_blob*)m_openBlobs[id]);
    }

    return 0;
}

// ============================================================ //

bool Storage::addContact(const UBJ::Object &obj)
{
    uint32_t contactId = obj["contactId"].toInt();

    std::string label = obj["label"].toString();

    Storage::NODE node = getNode(UBJ_OBJ("type" << Node::ContactType << "name" << label));

    if (!node) {

        // add new contact

        node = Node::create(Node::ContactType, 0, label);

        if (!node) {

            return false;
        }

        node->setUser1(contactId);

        node->setUser3(obj["phone"].toString());

        node->setUser4(obj["label2"].toString());

        if (obj.hasField("publicKey")) {

            UBJ::Object body;

            body["publicKey"] = obj["publicKey"];

            node->setBodyUbj(body);
        }

        if (!addNode(node)) {

            return false;
        }

        return true;
    }
    else
    if (contactId && obj.hasField("publicKey")) {

        // update id and public key of the contact

        node->setUser1(contactId);

        UBJ::Object body;

        node->bodyUbj(body);

        body["publicKey"] = obj["publicKey"];

        node->setBodyUbj(body);

        if (!updateNode(node)) {

            return false;
        }

        return true;
    }

    return false;
}

// ============================================================ //

bool Storage::getContact(uint32_t id, UBJ::Object &res)
{
    NODE node = getNode(UBJ_OBJ("type" << Node::ContactType << "user1" << id));

    if (!node) {

        return false;
    }

    UBJ::Object body;

    if (!node->bodyUbj(body)) {

        return false;
    }

    UBJ::Object contact;
    contact["contactId"] = node->user1();
    contact["label"]     = node->name();
    contact["phone"]     = node->user3();
    contact["publicKey"] = body["publicKey"];

    res = contact;

    return true;
}

// ============================================================ //

bool Storage::getContact(const std::string& label, UBJ::Object &res)
{
    NODE node = getNode(UBJ_OBJ("type" << Node::ContactType << "name" << label));

    if (!node) {

        return false;
    }

    UBJ::Object body;

    if (!node->bodyUbj(body)) {

        return false;
    }

    UBJ::Object contact;
    contact["contactId"] = node->user1();
    contact["label"]     = node->name();
    contact["phone"]     = node->user3();
    contact["publicKey"] = body["publicKey"];

    res = contact;

    return true;
}

// ============================================================ //

Storage::NODE_LIST Storage::getContacts(const UBJ::Object &query)
{
    UBJ::Value q = query;

    q["type"] = Node::ContactType;

    return getNodes(q);
}

// ============================================================ //

bool Storage::deleteContact(uint32_t id)
{
    UBJ::Object query = UBJ_OBJ("user1" << id << "type" << Node::ContactType);

    if (!deleteNode(query)) {

        return false;
    }

    return true;
}

// ============================================================ //

bool Storage::addRequest(const UBJ::Object &obj)
{
    NODE node = Node::create(Node::RequestType);

    if (!obj.hasField("requestId")) {

        return false;
    }

    node->setId(obj["requestId"].toInt());

    node->setBodyUbj(obj);

    addNode(node);

    return true;
}

// ============================================================ //

bool Storage::deleteRequest(uint32_t id)
{
    UBJ::Object query = UBJ_OBJ("id" << id << "type" << Node::RequestType);

    if (!deleteNode(query)) {

        return false;
    }

    return true;
}

// ============================================================ //

bool Storage::getRequest(uint32_t id, UBJ::Object &request)
{
    NODE node = getNode(UBJ_OBJ("id" << id << "type" << Node::RequestType));

    if (!node) {

        return false;
    }

    if (!node->bodyUbj(request)) {

        return false;
    }

    return true;
}

// ============================================================ //

Storage::NODE_LIST Storage::getRequests()
{
    return getNodes(UBJ_OBJ("type" << Node::RequestType));
}

// ============================================================ //

Storage::NODE Storage::createDirectory(const std::string& name, uint32_t parent)
{
    NODE node = Node::create(Node::DirectoryType, 0, name);

    if (node) {

        if (parent) {

            node->setParent(parent);
        }
        else {

            node->setParent(getNode(UBJ_OBJ("id" << VfsNodeId))->id());
        }

        if (addNode(node)) {

            return node;
        }
    }

    return nullptr;
}

// ============================================================ //

uint32_t Storage::createHistory(uint32_t contactId)
{
    NODE node = Node::create(Node::HistoryType);

    node->setTime(time(nullptr));

    node->setUser1(contactId);

    addNode(node);

    return node->id();
}

// ============================================================ //

uint32_t Storage::latestHistory(uint32_t contactId)
{
    NODE node = getNode(UBJ_OBJ("user1" << contactId << "type" << Node::HistoryType), UBJ_OBJ("rowid" << -1));

    uint32_t historyId = 0;

    if (node) {

        historyId = node->id();
    }
    else {

        historyId = createHistory(contactId);
    }

    return historyId;
}

// ============================================================ //

Storage::NODE Storage::storeMessage(MESSAGE msg)
{
    NODE node = Node::create(Node::MessageType);

    node->setId(msg->id());

    node->setTime(msg->time());

    node->setParent(msg->history());

    node->setUser1(msg->src());

    node->setUser2(msg->dst());

    UBJ::Object head;

    head["status"] = msg->status();

    head["userData"] = msg->value("userData");

    node->setHeadUbj(head);

    UBJ::Object body;

    body["text"] = msg->firstText();

    node->setBodyUbj(body);

    addNode(node);

    return node;
}

// ============================================================ //

bool Storage::updateMessage(MESSAGE msg)
{
    NODE node = getNode(UBJ_OBJ("id" << msg->id() << "type" << Node::MessageType));

    if (!node) {

        return false;
    }

    UBJ::Object update;

    update["time"] = msg->time();

    UBJ::Object head;

    node->headUbj(head);

    head["status"] = msg->status();

    head["userData"] = msg->value("userData");

    node->setHeadUbj(head);

    UBJ::Object body;

    node->bodyUbj(body);

    body["text"] = msg->firstText();

    node->setBodyUbj(body);

    if (!updateNode(node->id(), update << "head" << node->head() << "body" << node->body())) {

        return false;
    }

    return true;
}

// ============================================================ //

MESSAGE_LIST Storage::getMessages(uint32_t historyId)
{
    MESSAGE_LIST res;

    NODE_LIST nodes = getNodes(UBJ_OBJ("parent" << historyId << "type" << Node::MessageType));

    for (auto &node : nodes) {

        MESSAGE message = Message::create();

        message->setId(node->id());

        message->setTime(node->time());

        message->setSrc(node->user1());

        message->setDst(node->user2());

        UBJ::Object head;

        node->headUbj(head);

        message->setStatus((Message::Status)head["status"].toInt());

        (*message)["userData"] = head["userData"];

        if (node->body()) {

            UBJ::Object body;

            node->bodyUbj(body);

            if (body.hasField("text")) {

                RESOURCE res = Resource::createFromText("text", body["text"].toString());

                message->addResource(res);
            }
        }

        res.push_back(message);
    }

    return res;
}

// ============================================================ //

Storage::NODE Storage::storeResource(RESOURCE res, MESSAGE msg, uint32_t blankSpace, uint32_t parent)
{
    NODE node = Node::create(Node::ResourceType);

    if (res->id()) {

        node->setId(res->id());
    }
    else {

        node->setId(Crypto::mkId());
    }

    node->setName(res->name());

    node->setUser1(res->type());

    if (msg) {

        node->setUser2(msg->id());
    }

    if (parent) {

        node->setParent(parent);
    }
    else {

        // TODO make mind up about storing politics here

        node->setParent(VfsNodeId);
    }

    // set head

    UBJ::Object head;

    head["status"] = 0;

    head["userData"] = res->value("userData");

    node->setHeadUbj(head);

    // set body

    if (blankSpace > 0) {

        node->setBody(nullptr, blankSpace);
    }
    else {

        node->setBody(res->data());
    }

    // set hash

    node->setUser3(Crypto::hexStr(res->md5()));

    // add node

    if (!addNode(node)) {

        return nullptr;
    }

    return node;
}

// ============================================================ //

RESOURCE Storage::getResource(const UBJ::Object &query)
{
    UBJ::Object q = query;

    q["type"] = Node::ResourceType;

    NODE node = getNode(q);

    if (node && m_openBlobs.find(node->id()) == m_openBlobs.end()) {

        RESOURCE res = Resource::create();

        res->setId(node->id());

        res->setType((Zway::Resource::Type)node->user1());

        res->setName(node->name());

        res->setData(node->body());

        UBJ::Object head;

        node->headUbj(head);

        (*res)["userData"] = head["userData"];

        return res;
    }

    return nullptr;
}

// ============================================================ //

RESOURCE Storage::getResource(uint32_t id)
{
    return getResource(UBJ_OBJ("id" << id));
}

// ============================================================ //

RESOURCE_LIST Storage::getResources()
{
    RESOURCE_LIST ret;

    NODE_LIST nodes = getNodes(UBJ_OBJ("type" << Node::ResourceType));

    for (auto &node : nodes) {

        RESOURCE res = getResource(node->id());

        if (res) {

            ret.push_back(res);
        }
        else {

            res = Resource::create();

            res->setId(node->id());

            res->setType((Zway::Resource::Type)node->user1());

            res->setName(node->name());

            res->setData(node->body());

            ret.push_back(res);
        }
    }

    return ret;
}

// ============================================================ //

uint32_t Storage::incomingDir(uint32_t contactId)
{
    UBJ::Object contact;

    if (!getContact(contactId, contact)) {

        return 0;
    }

    NODE node = getNode(UBJ_OBJ("parent" << VfsNodeId << "name" << "Incoming"));

    if (!node) {

        node = createDirectory("Incoming", VfsNodeId);
    }

    if (node) {

        std::string contactLabel = contact["label"].toString();

        NODE dir = getNode(UBJ_OBJ("parent" << node->id() << "name" << contactLabel));

        if (!dir) {

            dir = createDirectory(contactLabel, node->id());
        }

        if (dir) {

            return dir->id();
        }
    }

    return 0;
}

// ============================================================ //

uint32_t Storage::outgoingDir(uint32_t contactId)
{
    UBJ::Object contact;

    if (!getContact(contactId, contact)) {

        return 0;
    }

    NODE node = getNode(UBJ_OBJ("parent" << VfsNodeId << "name" << "Outgoing"));

    if (!node) {

        node = createDirectory("Outgoing", VfsNodeId);
    }

    if (node) {

        std::string contactLabel = contact["label"].toString();

        NODE dir = getNode(UBJ_OBJ("parent" << node->id() << "name" << contactLabel));

        if (!dir) {

            dir = createDirectory(contactLabel, node->id());
        }

        if (dir) {

            return dir->id();
        }
    }

    return 0;
}

// ============================================================ //

bool Storage::getConfig(UBJ::Object &config)
{
    NODE node = getNode(UBJ_OBJ("id" << ConfigNodeId), UBJ::Object(), UBJ::Object(), 0, true, true);

    if (!node) {

        return false;
    }

    if (!node->bodyUbj(config)) {

        return false;
    }

    return true;
}

// ============================================================ //

bool Storage::setConfig(const UBJ::Object &config)
{
    NODE node = getNode(UBJ_OBJ("id" << ConfigNodeId), UBJ::Object(), UBJ::Object(), 0, true, true);

    if (!node) {

        return false;
    }

    UBJ::Object conf;

    node->bodyUbj(conf);

    std::list<std::string> settings = {
            "findByLabel",
            "findByPhone",
            "notifyStatus"};

    for (auto &s : settings) {

        if (config.hasField(s)) {

            conf[s] = config[s];
        }
    }

    node->setBodyUbj(conf);

    if (!updateNodeBody(node)) {

        return false;
    }

    return true;
}

// ============================================================ //

void Storage::cleanup()
{
    deleteNodes(
            UBJ_OBJ("type" << UBJ_ARR(
                Node::HistoryType <<
                Node::MessageType <<
                Node::ResourceType)));
}

// ============================================================ //

uint32_t Storage::version()
{
    NODE root = getNode(UBJ_OBJ("id" << RootNodeId), UBJ::Object(), UBJ_ARR("user1"), 0, false);

    if (root) {

        return root->user1();
    }

    return 0;
}

// ============================================================ //

std::string Storage::fieldsToReturnPart(const UBJ::Value &fieldsToReturn)
{
    std::string res;

    for (auto it = fieldsToReturn.cbegin(); it != fieldsToReturn.cend(); ) {

        res += it->second.toString();

        if (++it != fieldsToReturn.cend()) {

            res += ",";
        }
    }

    return res;
}

// ============================================================ //

std::string Storage::wherePart(const UBJ::Object &query)
{
    std::string res;

    for (auto it = query.cbegin(); it != query.cend(); ) {

        const UBJ::Value &v = it->second;

        if (v.type() == UBJ_ARRAY && v.numValues()) {

            res += "(";

            for (uint32_t i=0; i<v.numValues(); ++i) {

                res += it->first + "=?";

                if (i < v.numValues() - 1) {

                    res += " OR ";
                }
            }

            res += ")";
        }
        else {

            res += it->first + "=?";
        }

        if (++it != query.cend()) {

            res += " AND ";
        }
    }

    return res;
}

// ============================================================ //

std::string Storage::orderPart(const UBJ::Object &order)
{
    std::string res;

    for (auto it = order.cbegin(); it != order.cend(); ++it) {

        res += it->first + (it->second.toInt() > 0 ? " ASC " : " DESC ");
    }

    return res;
}

// ============================================================ //

std::string Storage::insertPart(const UBJ::Object &insert)
{
    std::string part1;
    std::string part2;

    for (auto it = insert.cbegin(); it != insert.cend(); ) {

        part1 += it->first;
        part2 += "?";

        if (++it != insert.cend()) {

            part1 += ",";
            part2 += ",";
        }
    }

    std::string res;

    if (!part1.empty()) {

        res = "(" + part1 + ") VALUES(" + part2 + ")";
    }

    return res;
}

// ============================================================ //

std::string Storage::updatePart(const UBJ::Object &update)
{
    std::string res;

    for (auto it = update.cbegin(); it != update.cend(); ) {

        res += it->first + "=?";

        if (++it != update.cend()) {

            res += ",";
        }
    }

    return res;
}

// ============================================================ //

Storage::Action Storage::prepareSelect(
        const std::string& table,
        const UBJ::Object &query,
        const UBJ::Object &order,
        const UBJ::Array &fieldsToReturn,
        int32_t limit,
        int32_t offset,
        bool encrypt)
{
    std::stringstream sql;

    sql << "SELECT ";

    std::string fieldsToReturnStr = fieldsToReturnPart(fieldsToReturn);

    if (!fieldsToReturnStr.empty()) {

        sql << fieldsToReturnStr << " FROM " << table;
    }
    else {

        sql << "* FROM " << table;
    }

    std::string whereStr = wherePart(query);

    if (!whereStr.empty()) {

        sql << " WHERE " << whereStr;
    }

    std::string orderStr = orderPart(order);

    if (!orderStr.empty()) {

        sql << " ORDER BY " << orderStr;
    }

    if (limit > 0) {

        sql << " LIMIT " << limit;
    }

    if (offset > 0) {

        sql << " OFFSET " << offset;
    }

    //Client::log("prepareSelect sql: %s", sql.str().c_str());

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2((sqlite3*)m_db, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {

        //Client::log("SQL Error: %s", sqlite3_errmsg((sqlite3*)m_db));

        return Action(nullptr, UBJ::Object());
    }

    Action action(stmt, query);

    bindUbjToStmt(stmt, action.data1(), 0, encrypt);

    return action;
}

// ============================================================ //

Storage::Action Storage::prepareCount(
        const std::string &table,
        const UBJ::Object &query,
        bool encrypt)
{
    std::stringstream sql;

    sql << "SELECT COUNT(1) FROM " << table;

    std::string whereStr = wherePart(query);

    if (!whereStr.empty()) {

        sql << " WHERE " << whereStr;
    }

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2((sqlite3*)m_db, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {

        return Action(nullptr, UBJ::Object());
    }

    Action action(stmt, query);

    bindUbjToStmt(stmt, action.data1(), 0, encrypt);

    return action;
}

// ============================================================ //

Storage::Action Storage::prepareInsert(
        const std::string &table,
        const UBJ::Object &insert,
        bool encrypt)
{
    std::stringstream sql;

    sql << "INSERT INTO " << table << " " << insertPart(insert);

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2((sqlite3*)m_db, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {

        return Action(nullptr, UBJ::Object());
    }

    Action action(stmt, insert);

    bindUbjToStmt(stmt, action.data1(), 0, encrypt);

    return action;
}

// ============================================================ //

Storage::Action Storage::prepareUpdate(
        const std::string &table,
        const UBJ::Object &update,
        const UBJ::Object &where,
        bool encrypt)
{
    std::stringstream sql;

    sql << "UPDATE " << table << " SET " << updatePart(update);

    std::string whereStr = wherePart(where);

    if (!whereStr.empty()) {

        sql << " WHERE " << whereStr;
    }

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2((sqlite3*)m_db, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {

        return Action(nullptr, UBJ::Object());
    }

    Action action(stmt, update, where);

    bindUbjToStmt(stmt, action.data1(), 0, encrypt);

    if (!whereStr.empty()) {

        bindUbjToStmt(stmt, action.data2(), update.numValues(), encrypt);
    }

    return action;
}

// ============================================================ //

Storage::Action Storage::prepareDelete(
        const std::string &table,
        const UBJ::Object &query,
        bool encrypt)
{
    std::stringstream sql;

    sql << "DELETE FROM " << table << " WHERE " << wherePart(query);

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2((sqlite3*)m_db, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {

        return Action(nullptr, UBJ::Object());
    }

    Action action(stmt, query);

    bindUbjToStmt(stmt, action.data1(), 0, encrypt);

    return action;
}

// ============================================================ //

int32_t Storage::bindUbjToStmt(
        void *stmt,
        const UBJ::Object &args,
        int32_t offset,
        bool encrypt)
{
    Crypto::AES aes;

    BUFFER ctr = Buffer::create(nullptr, 16);

    if (encrypt) {

        aes.setKey(m_key);
    }

    int32_t i=0;

    for (auto it = args.cbegin(); it != args.cend(); ++it) {

        const UBJ::Value &value = it->second;

        BUFFER buf = value.buffer();

        if (value.isValid()) {

            if (value.type() == UBJ_ARRAY) {

                if (value.size()) {

                    if (encrypt) {
                        aes.setCtr(ctr);
                        aes.encrypt(buf, buf, buf->size());
                    }

                    sqlite3_bind_blob((sqlite3_stmt*)stmt, ++i + offset, buf->data(), buf->size(), SQLITE_STATIC);
                }
                else {

                    int32_t res = bindUbjToStmt(stmt, value, i, encrypt);

                    i += res;
                }
            }
            else
            if (value.type() == UBJ_INT32) {

                if (encrypt) {
                    aes.setCtr(ctr);
                    aes.encrypt(buf, buf, buf->size());
                }

                sqlite3_bind_int((sqlite3_stmt*)stmt, ++i + offset, *((int*)buf->data()));
            }
            else
            if (value.type() == UBJ_STRING) {

                if (encrypt) {
                    aes.setCtr(ctr);
                    aes.encrypt(buf, buf, buf->size());
                }

                sqlite3_bind_text((sqlite3_stmt*)stmt, ++i + offset, (char*)buf->data(), buf->size(), SQLITE_STATIC);
            }
        }
        else {

            sqlite3_bind_null((sqlite3_stmt*)stmt, ++i + offset);
        }
    }

    return i;
}

// ============================================================ //

Storage::NODE Storage::makeNode(const UBJ::Object &meta, void *stmt, bool decrypt, bool secure)
{
    Crypto::AES aes;

    aes.setKey(m_key);

    BUFFER ctr = Buffer::create(nullptr, 16);

    NODE node = Node::create();

    node->setId(meta["id"].toInt());

    node->setTime(meta["time"].toInt());

    node->setType(meta["type"].toInt());

    node->setParent(meta["parent"].toInt());

    node->setName(meta["name"].toString());

    node->setUser1(meta["user1"].toInt());

    node->setUser2(meta["user2"].toInt());

    node->setUser3(meta["user3"].toString());

    node->setUser4(meta["user4"].toString());

    uint32_t headSize = sqlite3_column_bytes((sqlite3_stmt*)stmt, 9);

    uint32_t bodySize = sqlite3_column_bytes((sqlite3_stmt*)stmt, 10);

    if (headSize) {

        BUFFER buf;

        if (secure) {

            buf = Buffer::create((uint8_t*)sqlite3_column_blob((sqlite3_stmt*)stmt, 9), headSize);
        }
        else {

            buf = Buffer::create((uint8_t*)sqlite3_column_blob((sqlite3_stmt*)stmt, 9), headSize);
        }

        if (decrypt) {

            aes.setCtr(ctr);
            aes.decrypt(buf, buf, buf->size());
        }

        node->setHead(buf);
    }

    if (bodySize) {

        BUFFER buf;

        if (secure) {

            buf = Buffer::create((uint8_t*)sqlite3_column_blob((sqlite3_stmt*)stmt, 10), bodySize);
        }
        else {

            buf = Buffer::create((uint8_t*)sqlite3_column_blob((sqlite3_stmt*)stmt, 10), bodySize);
        }

        if (decrypt) {

            aes.setCtr(ctr);
            aes.decrypt(buf, buf, buf->size());
        }

        node->setBody(buf);
    }

    return node;
}

// ============================================================ //

void Storage::rowToUbj(void *stmt, UBJ::Object &obj, bool decrypt)
{
    Crypto::AES aes;

    BUFFER ctr = Buffer::create(nullptr, 16);

    if (decrypt) {

        aes.setKey(m_key);
    }

    int32_t numCols = sqlite3_column_count((sqlite3_stmt*)stmt);

    for (int32_t i=0; i<numCols; ++i) {

        int32_t type = sqlite3_column_type((sqlite3_stmt*)stmt, i);

        std::string name = sqlite3_column_name((sqlite3_stmt*)stmt, i);

        if (type == SQLITE_INTEGER) {

            int32_t v = sqlite3_column_int((sqlite3_stmt*)stmt, i);

            if (decrypt && name != "rowid") {

                BUFFER buf = Buffer::create((uint8_t*)&v, sizeof(v));

                aes.setCtr(ctr);
                aes.decrypt(buf, buf, sizeof(v));

                v = *((int*)buf->data());
            }

            obj[name] = v;
        }
        else
        if (type == SQLITE_TEXT) {

            int32_t len = sqlite3_column_bytes((sqlite3_stmt*)stmt, i);

            BUFFER buf = Buffer::create(sqlite3_column_text((sqlite3_stmt*)stmt, i), len);

            if (decrypt) {
                aes.setCtr(ctr);
                aes.decrypt(buf, buf, len);
            }

            obj[name] = std::string((char*)buf->data(), buf->size());
        }
        else
        if (type == SQLITE_BLOB) {

            int32_t len = sqlite3_column_bytes((sqlite3_stmt*)stmt, i);

            BUFFER buf = Buffer::create((uint8_t*)sqlite3_column_blob((sqlite3_stmt*)stmt, i), len);

            if (decrypt) {
                aes.setCtr(ctr);
                aes.decrypt(buf, buf, buf->size());
            }

            obj[name] = buf;
        }
    }
}

// ============================================================ //

// ============================================================ //
// Action
// ============================================================ //

Storage::Action::Action(void* stmt, const UBJ::Value &data1, const UBJ::Value &data2, const std::string &sql)
    : m_stmt(stmt),
      m_data1(data1.clone()),
      m_data2(data2.clone()),
      m_sql(sql)
{

}

// ============================================================ //

void* Storage::Action::stmt()
{
    return m_stmt;
}

// ============================================================ //

UBJ::Value &Storage::Action::data1()
{
    return m_data1;
}

// ============================================================ //

UBJ::Value &Storage::Action::data2()
{
    return m_data2;
}

// ============================================================ //

}
