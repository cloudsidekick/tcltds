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

#ifndef __SQLSERVERCONNECTION_H__
#define __SQLSERVERCONNECTION_H__

#ifndef SYBUNIQUE
enum
{
    SYBUNIQUE = 36     /* 0x24 */
};
#define SYBUNIQUE   SYBUNIQUE
#endif

#include <sqldb.h>
#include <string.h>

#include "dbconnection.h"

class TDSField : public DBField {
    public:
        TDSField();
        virtual ~TDSField();
        friend class SQLServerConnection;

        // Accessors
        virtual string getName() { return m_name; };
        virtual int getFieldLength() { return m_fieldLength; };
        virtual int getDataLength() { return m_dataLength; };
        virtual int getType() { return m_type; };

        // Returns the raw data stored by fetchRow()
        virtual void *getData() { return m_data; };

        // The following functions return formatted copies of the data
        virtual string getStringData();
        virtual int getIntData();


    private:
        string m_name;
        int m_fieldLength;
        int m_dataLength;
        int m_type;
        void *m_data;
        DBPROCESS *m_dbproc;
};

class SQLServerConnection : public DBConnection {
    public:
        SQLServerConnection(string servAddr, string uid, string pwd, 
                unsigned short port, string appName); 
        SQLServerConnection(DBConnection *a);
        ~SQLServerConnection();

        virtual int execute(string sql);
        virtual bool connect(string db);
        virtual DBRow *fetchRow();
        virtual string fetchRow(char *delim);
        virtual vector<string> getFieldList(string sql);
        virtual bool getIntSPReturn(int which, int *ret);
        virtual int getSPReturnStatus();
        virtual bool hasError() { return !m_error.empty(); };
        virtual void addError(string msg) { m_error += msg; };

        // Accessors
        string getAppName() { return m_appname; };

        // Manipulators
        void setAppName(string name) { m_appname = name; };

    private:
        DBPROCESS *m_dbproc;
        string m_error;
        int m_deadlock_counter;
};

int sybErrHandler(DBPROCESS *dbProc, int severity, int dbErr, int osErr, 
        char *dbErrStr, char *osErrStr);
int sybMsgHandler(DBPROCESS *dbProc, DBINT msgNo, int msgState, int severity,
        char *msgText, char *srvName, char *procName, int line);
#endif
