//########################################################################
// Copyright 2011 Cloud Sidekick
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//########################################################################

#ifndef __DBCONNECTION_H__
#define __DBCONNECTION_H__

#include <vector>
#include <map>
#include <string>
#include <iostream>

#include "utils.h"
using namespace std;

enum DBType {
    SQLServer,
    Oracle,
    SQLite,
    MySQL,
    PostgresQL
};

enum DBLibExceptionType {
    ECONNECT_FAILED,
    ECONVERSION_FAILED,
    ENO_CONNECTION,
    EPREPARE_FAILED,
    EEXECUTE_FAILED,
    EGENERAL_EXCEPTION,
    BCPINIT_FAILED,
    BCPSENDROW_FAILED,
    BCPBATCH_FAILED,
    BCPBIND_FAILED,
    BCPDONE_FAILED
};

class DBLibException {
    public:
        DBLibException(DBLibExceptionType type) {
            m_type = type;
            m_errInfo = "Unknown Error";
        };

        DBLibException(DBLibExceptionType type, string errInfo) {
            m_type = type;
            m_errInfo = errInfo;
        };

        ~DBLibException() {};

        DBLibExceptionType getType() { return m_type; };
        string getErrInfo() { return m_errInfo; };
        void addErrInfo(string err) { m_errInfo += err; };

    private:
        DBLibExceptionType m_type;
        string m_errInfo;
};

// This is a generic field type that the user will work with.
// For each DB type that we support, an extension to this class 
// should be defined that implements field behavor specific to
// that DB.
class DBField {
    public:
        DBField() {};
        virtual ~DBField() {};

        // Accessors
        virtual string getName() = 0;
        virtual int getFieldLength() = 0;
        virtual int getDataLength() = 0;
        virtual int getType() = 0;
        virtual void *getData() = 0;
        virtual void clear() {};

        // Data retrieval
        virtual string getStringData() = 0;
        virtual int getIntData() = 0;
};

class DBRow : public vector<DBField *> {
    public:
        DBRow();
        virtual ~DBRow();
        DBRow(DBRow *c);

        DBField *getField(string name);
        DBField *getField(int index);
};

// This is a generic DB conntion that is the interface for the
// user.  For each DB type that we support and extension to this
// class should be defined that implents DB specific behavior.
class DBConnection {
    public:
        virtual ~DBConnection() {};

        // The proper use of this function is:
        // DBConnection *dbc = DBConnection::buildConnection(...)
        // NEVER:
        // DBConnection *dbc = new DBConnection();
        // dbc->buildConnection(...).
        // In fact, DBConnection *dbc = new DBConnection(); will not compile
        // given that the constructor for this class is protected.
        static DBConnection *buildConnection(string connString);
        static DBConnection *buildConnection(DBType type, string servAddr, string uid, 
                string pwd, unsigned short port = 0, string appName = "");

        // These pure virtual functions must be overriden in each subclass
        virtual int execute(string sql) = 0;
        virtual bool connect(string db) = 0;
        virtual DBRow *fetchRow() = 0;
        virtual string fetchRow(char *delim) = 0;
        virtual vector<string> getFieldList(string sql) = 0;

        string getAddress() { return m_address; };
        string getUid() { return m_uid; };
        string getPwd() { return m_pwd; };
        string getApp() { return m_appname; };
        string getDb() { return m_db; };
        unsigned short getPort() { return m_port; };

        // Override this function if you need special escape handling in your DB type
        virtual string sqlEscape(string sql);

        void printRow(DBRow *theRow);

    protected:
        // Prevent the constructor from ever being called
        DBConnection() {};

        string m_address;
        string m_uid;
        string m_pwd;
        string m_db;
        unsigned short m_port;
        string m_appname;

};

#endif
