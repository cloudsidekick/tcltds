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

#include <sstream>
#include <algorithm>
#include <stdio.h>

#include "sqlserverconnection.h"

// *************************************************************
//
// TDSField Implementation
//
// *************************************************************

//DBLibException *e = NULL;
SQLServerConnection *connPtr;

TDSField::TDSField() {
    m_name = "";
    m_fieldLength = 0;
    m_dataLength = 0;
    m_type = -1;
    m_data = NULL;
}

TDSField::~TDSField() {
    delete[] (char *)m_data;
}

string
TDSField::getStringData() {
    string retval;

    char *tempChar = NULL;
    switch (getType()) {
        case SYBDATETIME:
        case SYBDATETIME4:
            tempChar = new char[32];
            memset(tempChar, 0x0, 32);
            break;
        case SYBTEXT:
        case SYBVARCHAR:
        case SYBCHAR:
            tempChar = new char[getDataLength() + 1];
            memset(tempChar, 0x0, getDataLength() + 1);
            break;
        case SYBBINARY:
        case SYBVARBINARY:
        case SYBIMAGE:
        case SYBINT1:
        case SYBINT2:
        case SYBINT4:
        case SYBINT8:
        case SYBBIT:
        case SYBMONEY:
        case SYBMONEY4:
        case SYBNUMERIC:
        case SYBDECIMAL:
        case SYBFLT8:
        case SYBREAL:
            tempChar = new char[64];
            memset(tempChar, 0x0, 64);
            break;
    }

    if (!dbwillconvert(getType(), SYBCHAR)) {
        stringstream err;
        err << "Could not convert type [" << getType() << "] to SYBCHAR";
        dbcancel(m_dbproc);
        throw DBLibException(ECONVERSION_FAILED, err.str());
    }

    dbconvert(m_dbproc, getType(), (BYTE *)getData(), getDataLength(), SYBCHAR,
            (BYTE *)tempChar, (DBINT)-1);

    retval = tempChar;
    delete[] tempChar;

    return retval;
}

int
TDSField::getIntData() {
    int tempInt;
    
    if (!dbwillconvert(getType(), SYBINT4)) {
        stringstream err;
        err << "Could not convert type [" << getType() << "] to SYBINT4";
        dbcancel(m_dbproc);
        throw DBLibException(ECONVERSION_FAILED, err.str());
    }

    dbconvert(m_dbproc, getType(), (BYTE *)getData(), getDataLength(), SYBINT4,
            (BYTE *)&tempInt, (DBINT)sizeof(int));

    return tempInt;
}

// *************************************************************
//
// SQLServerConnection Implementation
//
// *************************************************************

bool
SQLServerConnection::getIntSPReturn(int which, int *ret) {
    while (dbnextrow(m_dbproc) != NO_MORE_ROWS);

    if (!dbhasretstat(m_dbproc))
        return false;
    
    if (which == 0 || which > dbnumrets(m_dbproc))
        return false;
    
    *ret = *(int *)dbretdata(m_dbproc, which);

    return true;
}

int
SQLServerConnection::getSPReturnStatus() {
    while (dbnextrow(m_dbproc) != NO_MORE_ROWS);

    return dbretstatus(m_dbproc);
    
}

SQLServerConnection::SQLServerConnection(string servAddr, string uid, string pwd, 
        unsigned short port, string appName) {

    char hostPort[512] = "";
    m_uid = uid;
    m_pwd = pwd;

    if (port > 0)
        m_port = port;
    else
        m_port = 1433;

    sprintf(hostPort, "%s:%hu", servAddr.c_str(), m_port);
    m_address = hostPort;
    m_appname = appName;
    m_dbproc = NULL;
    m_db = "";
    m_error = "";
    m_deadlock_counter = 0;
}

SQLServerConnection::SQLServerConnection(DBConnection *c) {
    m_address = c->getAddress();
    m_uid = c->getUid();
    m_pwd = c->getPwd();
    m_port = c->getPort();
    m_appname = c->getApp();
    m_db = c->getDb();
    m_dbproc = NULL;
    m_error = "";
    m_deadlock_counter = 0;

    connect(m_db);
}

SQLServerConnection::~SQLServerConnection() { 
    if (m_dbproc) {
        dbclose(m_dbproc); 
        m_dbproc = NULL;
    }
}

bool
SQLServerConnection::connect(string db = "") {
    connPtr = this;
    LOGINREC *login;

    m_error.clear();
    if (!m_dbproc) {
        dbinit();
        dberrhandle(sybErrHandler);
        dbmsghandle(sybMsgHandler);

        login = dblogin();

        DBSETLPWD(login, m_pwd.c_str());
        DBSETLUSER(login, m_uid.c_str());
        if (m_appname.length() > 0)
            DBSETLAPP(login, m_appname.c_str());
        DBSETLCHARSET(login, "iso_1");
        if (m_appname.length())
            DBSETLAPP(login, m_appname.c_str());
        m_dbproc = dbopen(login, (char *)m_address.c_str());
        dbloginfree(login);
        if (hasError()) 
            throw DBLibException(EGENERAL_EXCEPTION, m_error); 
    }

    if (!db.empty()) {
        m_db = db;
        dbuse(m_dbproc, (char *)db.c_str());
        if (hasError())
            throw DBLibException(EGENERAL_EXCEPTION, m_error); 
    }

    execute("set textsize 2147483000");

    return true;
}

int
SQLServerConnection::execute(string sql) {
    int retVal = 0;

    connPtr = this;
    m_error.clear();

    if (!m_dbproc) {
        string err;
        err = "SQLServerConnection::execute() called with no active connection";
        throw DBLibException(ENO_CONNECTION, err);
    }

    while (dbcmd(m_dbproc, sql.c_str()) != SUCCEED) {
        if (hasError()) {
            dbcancel(m_dbproc);
            addError("\n( dbcmd() ) SQL was: " + sql);
            throw DBLibException(EGENERAL_EXCEPTION, m_error); 
        }
    }

    while (dbsqlexec(m_dbproc) != SUCCEED) {
        if (hasError()) {
            dbcancel(m_dbproc);
            addError("\n( dbsqlexec() ) SQL was: " + sql);
            throw DBLibException(EGENERAL_EXCEPTION, m_error); 
        }
    }

    while (dbresults(m_dbproc) != NO_MORE_RESULTS) {
        if (hasError()) {
            dbcancel(m_dbproc);
            addError("\n( dbresults() ) SQL was: " + sql);
            throw DBLibException(EGENERAL_EXCEPTION, m_error); 
        }

        // If this command can return data (select * ....)
        if (DBCMDROW(m_dbproc) == SUCCEED) {
            // And it does return data
            if (DBROWS(m_dbproc) == SUCCEED)
                return 1;
            else // It doesn't return data
                return 0;
        }
        // The current command cannot return data.  It's an update, insert, etc.
        else {
            retVal += DBCOUNT(m_dbproc);
        }
    }

    m_deadlock_counter = 0;
    return retVal;
}

DBRow *
SQLServerConnection::fetchRow() {
    int colCount = 0;

    connPtr = this;
    m_error.clear();

    if (dbnextrow(m_dbproc) == NO_MORE_ROWS) {
        bool hasResults = false;
        if (DBMORECMDS(m_dbproc) == SUCCEED) {
            while (dbresults(m_dbproc) != NO_MORE_RESULTS) {
                if (hasError()) {
                    dbcancel(m_dbproc);
                    throw DBLibException(EGENERAL_EXCEPTION, m_error);
                }
                if (DBCMDROW(m_dbproc) == SUCCEED) {
                    if (DBROWS(m_dbproc) == SUCCEED) {
                        hasResults = true;
                        dbnextrow(m_dbproc);
                        break;
                    }
                }
            }
        }

        if (!hasResults)
            return NULL;
    }

    DBRow *newRow = new DBRow;

    colCount = dbnumcols(m_dbproc);

    for (int x = 1; x <= colCount; x++) {
        TDSField *newField = new TDSField();

        newField->m_name = dbcolname(m_dbproc, x);
        if (newField->m_name == "") {
            char tmp[32];
            sprintf(tmp, "unknown_%d", x);
            newField->m_name = tmp;
        }
        transform(newField->m_name.begin(), newField->m_name.end(), 
                newField->m_name.begin(), to_lower());
        newField->m_fieldLength = dbcollen(m_dbproc, x);
        newField->m_dataLength = dbdatlen(m_dbproc, x);
        newField->m_type = dbcoltype(m_dbproc, x);
        if (newField->m_type == SYBUNIQUE) {
            newField->m_type = SYBBINARY;
        }

        if (newField->m_dataLength > 0) {
            void *newData = new char[newField->m_dataLength];
            memcpy(newData, (void *)dbdata(m_dbproc, x), newField->m_dataLength);
            newField->m_data = newData;
        }
        else {
            // The field was NULL
            newField->m_data = NULL;
        }
        newField->m_dbproc = m_dbproc;
        newRow->push_back((newField));
    }

    return newRow;
}

string 
SQLServerConnection::fetchRow(char *delimit_char) {
    int colCount = -1;

    connPtr = this;

    if (dbnextrow(m_dbproc) == NO_MORE_ROWS) {
        colCount = -1;
        return "";
    }

    int valLen=0;
    string retval = "";
    DBCHAR tmp[65535];

    colCount = dbnumcols(m_dbproc);

    //memset(tmp, 0x0, 65535);
    for (int x = 1; x <= colCount; x++) {
        valLen = dbdatlen(m_dbproc, x);
        if (x>1)
            retval += delimit_char;
        if (valLen>0) {
            dbconvert(m_dbproc, dbcoltype(m_dbproc, x),  dbdata(m_dbproc, x), valLen, SYBCHAR, (BYTE *)tmp, (DBINT) -1);
            retval += tmp;
            //memset(tmp, 0x0, valLen);
        }
    }
    return retval;
}

int
sybErrHandler(DBPROCESS *dbProc, int severity, int dbErr, int osErr,
        char *dbErrStr, char *osErrStr) {
    string err;

    // It's OK to end up here in response to a dbconvert() that
    // resulted in overflow, so don't exit in that case.
    if (dbErr == SYBEICONVI)
        return INT_CANCEL;

    /*
    * For server messages, cancel the query and rely on the
    * message handler to spew the appropriate error messages out.
    */
    if (dbErr == SYBESMSG)
        return INT_CANCEL;

    if (dbErrStr)
        err = "\nDBLIB ERROR: [" + string(dbErrStr) + "]";

    if (osErrStr)
        err += "\nOS ERROR: [" + string(osErrStr) + "]";

    if (connPtr)
        connPtr->addError(err);

    if (dbErr == SYBETIME)
        return INT_TIMEOUT;
    else
        return INT_CANCEL;
}

int
sybMsgHandler(DBPROCESS *dbProc, DBINT msgNo, int msgState,
        int severity, char *msgText, char *srvName, char *procName, int line) {

    if (msgNo == 5701 || msgNo == 5703 || msgNo == 5704)
        return 0;

    stringstream err;
    err << "\nDBLIB MESSAGE: [msgno: " << msgNo << 
           ", level: " << severity << ", State: " << msgState << "]";
    if (*srvName)
        err << "\nSERVER: [" << srvName << "]";
    if (*procName)
        err << "\nPROCEDURE: [" << procName << "]";
    if (line > 0)
        err << "\nLINE: [" << line << "]";

    err << "\nMESSAGE: " << msgText;

    if (connPtr) {
        connPtr->addError(err.str());
    }
    
    return 0;
}

vector<string>
SQLServerConnection::getFieldList(string sql) {
    vector<string> fieldList;

    while (sql[0] == ' ')
        sql.erase(sql.begin());

    string queryType = sql.substr(0,6);
    transform(queryType.begin(), queryType.end(), queryType.begin(), to_upper());
    if (queryType != "SELECT")
        return fieldList;

    execute("set fmtonly on");

    execute(sql);

    int colCount = dbnumcols(m_dbproc);
    for (int x = 1; x <= colCount; x++)
        fieldList.push_back(dbcolname(m_dbproc, x));

    execute("set fmtonly off");

    return fieldList;
}
