
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

#ifndef STORAGE_H_
#define STORAGE_H_

#include "Zway/crypto/crypto.h"
#include "Zway/message/message.h"

#include <list>

namespace Zway {

extern const uint32_t STORAGE_VERSION;

// ============================================================ //

class Storage
{
public:

    class Node;

    typedef std::shared_ptr<Node> NODE;

    typedef std::list<NODE> NODE_LIST;

    class Action;

    enum {

        RootNodeId=1,

        DataNodeId,

        VfsNodeId,

        ConfigNodeId
    };

    typedef std::shared_ptr<Storage> Pointer;

    static Pointer init(const std::string &filename, const std::string &password, const UBJ::Value &data);

    static Pointer open(const std::string &filename, const std::string &password);

    ~Storage();

    void close();

    uint32_t accountId();

    uint32_t accountPw();

    std::string accountLabel();

    UBJ::Object &privateKey();

    UBJ::Object &publicKey();

    bool addNode(NODE node, bool encrypt=true);

    NODE getNode(
            const UBJ::Object &query,
            const UBJ::Object &order=UBJ::Object(),
            const UBJ::Array &fieldsToReturn=UBJ::Array(),
            int32_t offset=0,
            bool decrypt=true,
            bool secure=false);

    NODE_LIST getNodes(
            const UBJ::Object &query,
            const UBJ::Object &order=UBJ::Object(),
            const UBJ::Array &fieldsToReturn=UBJ::Array(),
            int32_t limit=0,
            int32_t offset=0,
            bool decrypt=true,
            bool secure=false);

    bool updateNode(uint32_t nodeId, const UBJ::Object &update, bool encrypt = true);

    bool updateNode(
            NODE node,
            const UBJ::Array &fieldsToUpdate=UBJ::Array(),
            bool updateHead=true,
            bool updateBody=true,
            bool encrypt=true);

    bool updateNodeHead(NODE node, bool encrypt=true);

    bool updateNodeBody(NODE node, bool encrypt=true);

    bool deleteNode(const UBJ::Object &query, bool deleteChildren=true, bool encryptQuery=true);

    bool deleteNodes(const UBJ::Object &query, bool deleteChildren=true, bool encryptQuery=true);


    uint32_t getNodeCount(const UBJ::Object &query, bool encrypt=true);


    bool openBodyBlob(uint32_t id, bool encryptQuery=true);

    bool closeBodyBlob(uint32_t id);

    bool readBodyBlob(uint32_t id, uint8_t* data, uint32_t size, uint32_t offset);

    bool writeBodyBlob(uint32_t id, uint8_t* data, uint32_t size, uint32_t offset);

    bool zeroBodyBlob(uint32_t id);

    uint32_t openBlobsSize(uint32_t id);


    bool addContact(const UBJ::Object &obj);

    bool getContact(uint32_t id, UBJ::Object &res);

    bool getContact(const std::string& label, UBJ::Object &res);

    bool deleteContact(uint32_t id);

    NODE_LIST getContacts(const UBJ::Object &query = UBJ::Object());



    bool addRequest(const UBJ::Object &obj);

    bool deleteRequest(uint32_t id);

    bool getRequest(uint32_t id, UBJ::Object &request);

    NODE_LIST getRequests();


    NODE createDirectory(const std::string& name, uint32_t parent=0);


    uint32_t createHistory(uint32_t contactId);

    uint32_t latestHistory(uint32_t contactId);


    NODE storeMessage(MESSAGE msg);

    bool updateMessage(MESSAGE msg);

    MESSAGE_LIST getMessages(uint32_t historyId);


    NODE storeResource(RESOURCE res, MESSAGE msg=nullptr, uint32_t blankSpace=0, uint32_t parent=0);

    RESOURCE getResource(const UBJ::Object &query);

    RESOURCE getResource(uint32_t id);

    RESOURCE_LIST getResources();


    uint32_t incomingDir(uint32_t contactId);

    uint32_t outgoingDir(uint32_t contactId);


    bool getConfig(UBJ::Object &config);

    bool setConfig(const Zway::UBJ::Object &config);


    void cleanup();

    uint32_t version();

private:

    Storage();

protected:

    bool _init(const std::string &filename, const std::string &password, const UBJ::Value &data);

    bool _open(const std::string &filename, const std::string &password);

    bool createDefaultNodes();

    std::string fieldsToReturnPart(const UBJ::Value &fieldsToReturn);

    std::string wherePart(const UBJ::Object &query);

    std::string orderPart(const UBJ::Object &order);

    std::string insertPart(const UBJ::Object &insert);

    std::string updatePart(const UBJ::Object &update);

    Action prepareSelect(
            const std::string& table,
            const UBJ::Object &query,
            const UBJ::Object &order=UBJ::Object(),
            const UBJ::Array &fieldsToReturn=UBJ::Array(),
            int32_t limit=0,
            int32_t offset=0,
            bool encrypt=true);

    Action prepareCount(
            const std::string &table,
            const UBJ::Object &query,
            bool encrypt=true);

    Action prepareInsert(
            const std::string &table,
            const UBJ::Object &insert,
            bool encrypt=true);

    Action prepareUpdate(
            const std::string &table,
            const UBJ::Object &update,
            const UBJ::Object &where,
            bool encrypt=true);

    Action prepareDelete(
            const std::string &table,
            const UBJ::Object &query,
            bool encrypt=true);

    int32_t bindUbjToStmt(
            void* stmt,
            const UBJ::Object &args,
            int32_t offset=0,
            bool encrypt=true);

    void rowToUbj(void* stmt, UBJ::Object &obj, bool decrypt=true);

    NODE makeNode(const UBJ::Object &data, void* stmt, bool decrypt, bool secure);

protected:

    void* m_db;

    uint32_t m_accountId;

    uint32_t m_accountPw;

    std::string m_accountLabel;

    BUFFER m_key;

    UBJ::Object m_privateKey;

    UBJ::Object m_publicKey;

    std::map<int, void*> m_openBlobs;

    std::map<int, Crypto::AES*> m_openBlobsAes;

    std::map<int, std::string> m_contactLabels;
};

typedef Storage::Pointer STORAGE;

// ============================================================ //

class Storage::Action
{
public:

    Action(void* stmt, const UBJ::Value &data1, const UBJ::Value &data2=UBJ::Object(), const std::string &sql = std::string());

    void* stmt();

    UBJ::Value &data1();

    UBJ::Value &data2();

protected:

    void* m_stmt;

    UBJ::Value m_data1;

    UBJ::Value m_data2;

    std::string m_sql;
};

// ============================================================ //

}

#include "Zway/storage/node.h"

#endif /* STORAGE_H_ */
