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

#include "dbconnection.h"
#include "sqlserverconnection.h"
#include <algorithm>

using namespace std;

// *************************************************************
//
// DBRow Implementation
//
// *************************************************************

DBRow::DBRow() {
    clear();
}

DBRow::DBRow(DBRow *c) {
    clear();
    DBRow::iterator vi = c->begin();
    for (; vi != c->end(); ++vi)
        push_back(*vi);
}

DBRow::~DBRow() {
    for (unsigned int x = 0; x < size(); x++)
        delete (*this)[x];
}

DBField *
DBRow::getField(string name) {
    for (unsigned int x = 0; x < size(); x++) {
        string fieldName = (*this)[x]->getName();
        transform(fieldName.begin(), fieldName.end(), fieldName.begin(), to_lower());
        transform(name.begin(), name.end(), name.begin(), to_lower());
        if (fieldName == name)
            return (*this)[x];
    }
    return NULL;
}

DBField *
DBRow::getField(int index) {
    if (index < 0 || index >= size())
        return NULL;

    return (*this)[index];
}

// *************************************************************
//
// DBConnection Implementation
//
// *************************************************************

// This function should currently take a string with the following format.
// The order of the parameters is not important, however, case is
// ADDRESS=192.168.1.1;UID=foo;PWD=bar;PORT=1433;CONNTYPE=SQLServer;DB=baz
// Port is optional, CONNTYPE is optional and defaults to SQLite, DB is
// optional and just specifies a database to connect to.  If DB is absent
// the connection is built but won't be "using" a database.
DBConnection *
DBConnection::buildConnection(string connString) {
    string servAddr;
    string uid;
    string pwd;
    string sPort;
    DBType type = SQLServer;
    string sType;
    string dbname;
    string appName;
    int port = 1433;

    servAddr = connString.substr(connString.find("ADDRESS=") + strlen("ADDRESS="));
    servAddr = servAddr.substr(0, servAddr.find(";"));

    uid = connString.substr(connString.find("UID=") + strlen("UID="));
    uid = uid.substr(0, uid.find(";"));

    pwd = connString.substr(connString.find("PWD=") + strlen("PWD="));
    pwd = pwd.substr(0, pwd.find(";"));

    sType = connString.substr(connString.find("CONNTYPE=") + strlen("CONNTYPE="));
    sType = sType.substr(0, sType.find(";"));
     
    if (connString.find("PORT=") != string::npos) {
        sPort = connString.substr(connString.find("PORT=") + strlen("PORT="));
        sPort = sPort.substr(0, sPort.find(";"));
        port = atoi(sPort.c_str());
    }

    if (connString.find("DB=") != string::npos) {
        dbname = connString.substr(connString.find("DB=") + strlen("DB="));
        dbname = dbname.substr(0, dbname.find(";"));
    }

    if (connString.find("APPNAME=") != string::npos) {
        appName = connString.substr(connString.find("APPNAME=") + strlen("APPNAME="));
        appName = appName.substr(0, appName.find(";"));
    }

    if (sType == "SQLServer")
        type = SQLServer;
    if (servAddr.empty() || uid.empty() || pwd.empty())
        return NULL;

    if (type == SQLServer) {
        SQLServerConnection *conn = new SQLServerConnection(servAddr, uid, pwd, port, appName);
        conn->connect(dbname);
        return conn;
    }

    return NULL;
}

DBConnection *
DBConnection::buildConnection(DBType type, string servAddr, string uid, string pwd, 
        unsigned short port, string appName) {
    if (type == SQLServer)
        return new SQLServerConnection(servAddr, uid, pwd, port, appName);
    return NULL;
}

string
DBConnection::sqlEscape(string sql) {
    string newSql;

    string::iterator si = sql.begin();
    for (; si != sql.end(); ++si) {
        if (*si == '\'')
            newSql += "''";
        else
            newSql += *si;
    }

    return newSql;
}

void
DBConnection::printRow(DBRow *theRow) {
    DBField *theField;

    for (unsigned x = 0; x < theRow->size(); x++) {
        theField = (*theRow)[x];
        if (x == 0)
            cout << theField->getStringData();
        else
            cout << " | " + theField->getStringData();
    }

    cout << endl;
}
