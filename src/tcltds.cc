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

#include "tcltds.h"
#include <vector>
#include <cstdlib>
#include <climits>
#include <algorithm>
#include <cstring>

using namespace std;

int tcltdsConnVal=1, tcltdsDefaultDelim=0;
char tcltdsDelimitChar[5];
vector<tcltdsConnObj> tcltdsConnVector;

DBConnection *
tcltdsGetConn(int cID) {
    if (tcltdsConnVector.empty()) {
        return NULL;
    }
    if (cID==0) {
        return tcltdsConnVector.back().conn;
    }
    for (int x=0; x<tcltdsConnVector.size(); x++) {
        if (tcltdsConnVector[x].connID == cID) {
            return tcltdsConnVector[x].conn;
        }
    }
    return NULL;
}
       
int 
tcltdsConnect(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]) {
    DBConnection *tcltdsDBConn;
    if (objc>=5) {
        char USER[256];
        char SERVER[256];
        char PASSWORD[256];
        char DATABASE[256];
        unsigned short PORT = 0;
        DBType DB_ENUM_VAL = SQLServer;
        char APPNAME[256] = "";
        string DBTypeStr; 
        char Tcl_retval[8];

        strcpy(USER,(char *)objv[1]);
        strcpy(PASSWORD,(char *)objv[2]);
        strcpy(SERVER,(char *)objv[3]);
        strcpy(DATABASE,(char *)objv[4]);
	if (strlen((char *)objv[5]) > 0) {
		int tmp_port = atoi((char *)objv[5]);
		if ((tmp_port > 0) && (tmp_port<=65534)) {
		    PORT = tmp_port;
		} else {
			Tcl_SetResult(interp, (char *)"ERROR: port parameter must be an integer between 1 and 65534", TCL_VOLATILE);
			return TCL_ERROR;
		}
	} else {
		PORT = 1433;
	}
	if (objc==7) {
	    strcpy(APPNAME,(char *)objv[6]);
	}

        //fprintf(stdout, "calling buildConnection");
        tcltdsDBConn = DBConnection::buildConnection(DB_ENUM_VAL, SERVER, USER, PASSWORD, PORT, APPNAME);
        //fprintf(stdout, "called buildConnection");
        if (tcltdsDBConn) {
            try {
                tcltdsDBConn->connect(DATABASE);
            }
            catch (DBLibException &e) {
                //fprintf(stdout, "ERROR: Database connection attempt failed: %s", e.getErrInfo().c_str());
                Tcl_SetResult(interp, (char *)e.getErrInfo().c_str(), TCL_VOLATILE);
                return TCL_ERROR;
            }
            //fprintf(stdout, "established db connection, tcltdsDBConn is set");
            tcltdsConnObj tmpConn;
            tmpConn.connID = tcltdsConnVal++;
            //fprintf(stdout, "tcltdsConnVal: %d", tcltdsConnVal);
            tmpConn.conn = tcltdsDBConn;
            tcltdsConnVector.push_back(tmpConn);
            //fprintf(stdout, "pushed new connection");
            //lastConnection = tcltdsConnVector.size()-1;
            sprintf(Tcl_retval, "%d", tmpConn.connID);
            //fprintf (stdout, "DEBUG: setting %d as connID", tmpConn.connID);
            //fprintf(stdout, "setting DB_CONN");
            //Tcl_SetVar(interp, "DB_CONN", Tcl_retval, TCL_GLOBAL_ONLY);
            Tcl_SetResult(interp, (char*)Tcl_retval, TCL_VOLATILE);
            //fprintf(stdout, "set DB_CONN");
            return TCL_OK;
        }
        //fprintf(stdout, "ERROR: Could not create database connection object\n"); 
        Tcl_SetResult(interp, (char*)"ERROR: Could not create database connection object", TCL_VOLATILE);
        return TCL_ERROR;
    }
    else {
        //fprintf(stdout, "ERROR: Invalid number of arguments to procedure \"connect\"\n"); 
        Tcl_SetResult(interp, (char*)"Invalid number of arguments to procedure \"tds_connect\"", TCL_VOLATILE);
        return TCL_ERROR;
    }
}

int 
tcltdsQuery(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]) {
    //fprintf(stdout, "DEBUG: in tcltdsQuery");
    vector<string> fieldList;
    string sql;
    DBConnection *tcltdsDBConn = tcltdsGetConn();

    if (objc==2) { // 1 parameter only, which was the query
        sql = (char *)objv[1];
    }
    else if (objc==3) { // 2 parameters, first connection ID, then query
        tcltdsDBConn = tcltdsGetConn(atoi((char *)objv[1]));
        sql = (char *)objv[2];
        //fprintf(stdout, "tcltdsDBConn ID is: %d", atoi((char *)objv[1]));
    }
    else {
        Tcl_SetResult(interp, (char*)"ERROR: Invalid number of arguments to tds_query", TCL_VOLATILE);
        return TCL_ERROR;
    }
    //fprintf(stdout, "sql is: %s", sql.c_str());
    if (tcltdsDBConn == NULL) {
        Tcl_SetResult(interp, (char*)"ERROR: Invalid connection id passed to tds_query", TCL_VOLATILE);
        return TCL_ERROR;
    }

    //fprintf(stdout, "DEBUG: connection should be selected");
    try {
        fieldList = tcltdsDBConn->getFieldList(sql);
    }
    catch (DBLibException &e) {
        Tcl_SetResult(interp, (char *)e.getErrInfo().c_str(), TCL_VOLATILE);
        return TCL_ERROR;
    }
    //fprintf(stdout, "DEBUG: getFieldList occurred properly");
    string allFields = "{";

    for (int x=0; x<fieldList.size(); x++) {
        if (x)
            allFields += "} {";
        allFields += fieldList[x];
    }
    allFields += "}";
    //Tcl_SetVar(interp, "DB_COL_NAMES", allFields.c_str(), TCL_GLOBAL_ONLY);
    if (fieldList.size() == 1)
        Tcl_SetResult(interp, (char *)fieldList[0].c_str(), TCL_VOLATILE);
    else
        Tcl_SetResult(interp, (char *)allFields.c_str(), TCL_VOLATILE);
    try {
        //fprintf(stdout, "DEBUG: trying to execute");
        int rc = tcltdsDBConn->execute(sql);
        //fprintf(stdout, "DEBUG: successfully execute: %d", rc);
    }
    catch (DBLibException &e) {
        Tcl_SetResult(interp, (char *)e.getErrInfo().c_str(), TCL_VOLATILE);
        return TCL_ERROR;
    }
    //fprintf(stdout, "DEBUG: exit tcltdsQuery");
    return TCL_OK;
}

int 
tcltdsFetchRow(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]) {
    DBConnection *tcltdsDBConn = tcltdsGetConn();
    //since both params (db conn ID and delimiter) are optional, conn ID takes priority, user must send conn ID if they want a delimiter
    if (objc == 2) { // 1 parameter passed, which is the connID
        //int connID = tcltdsConnVector.back().connID;
        //fprintf (stdout, "DEBUG: using %d as connID", connID);
        tcltdsDBConn = tcltdsGetConn(atoi((char *)objv[1]));
        //fprintf(stdout, "tcltdsDBConn ID = %d", atoi((char *)objv[1]));
    }
    else if (objc == 3) { // 2 parameters passed, first is database conn ID, then delimiter
        tcltdsDBConn = tcltdsGetConn(atoi((char *)objv[1]));
        //fprintf(stdout, "tcltdsDBConn ID = %d", atoi((char *)objv[1]));
        strncpy(tcltdsDelimitChar,(char *)objv[2],5);
    }
    else if (objc > 3) {
        fprintf(stdout, "WARNING: Invalid number of arguments to tds_fetchrow, ignoring extra");
    }
    if (tcltdsDBConn == NULL) {
        Tcl_SetResult(interp, (char*)"ERROR: Invalid connection id pased to tds_fetchrow", TCL_VOLATILE);
        return TCL_ERROR;
    }
    if (strlen(tcltdsDelimitChar)==0) {
        strcpy(tcltdsDelimitChar, "} {"); // temporary to test
        tcltdsDefaultDelim = 1;
    }
    string tcl_retval = tcltdsDBConn->fetchRow((char *)tcltdsDelimitChar);
    //if (tcltdsDefaultDelim && tcl_retval.length() > 0) {
        if (tcl_retval.find(tcltdsDelimitChar) != string::npos)
            tcl_retval = "{" + tcl_retval + "}";
    //}
    Tcl_SetResult(interp, (char *)tcl_retval.c_str(), TCL_VOLATILE);
    //fprintf(stdout, "DEBUG: returning from  tcltdsFetchRow");
    return TCL_OK;
}

int 
tcltdsFetchSet(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]) {
    DBConnection *tcltdsDBConn = tcltdsGetConn();
    //fprintf(stdout, "DEBUG: in tcltdsFetchSet");
    if (objc == 2) { // 1 parameter passed, connID
        tcltdsDBConn = tcltdsGetConn(atoi((char *)objv[1]));
        //fprintf(stdout, "tcltdsDBConn ID = %d", atoi((char *)objv[1]));
    }
    else if (objc > 2) {
        fprintf(stdout, "WARNING: Invalid number of arguments to tds_fetchset, ignoring extra");
    }
    if (tcltdsDBConn == NULL) {
        Tcl_SetResult(interp, (char*)"ERROR: Invalid connection id pased to tds_fetchset", TCL_VOLATILE);
        return TCL_ERROR;
    }
    string tcl_retval = "";
    strcpy(tcltdsDelimitChar, "} {"); // temporary to test
    //string tclRetVal = "";
    string rowBuf = "";
    while ((rowBuf = tcltdsDBConn->fetchRow((char *)tcltdsDelimitChar)) != "") {
        if (tcl_retval.length()>1) 
            tcl_retval += " ";
        tcl_retval += "{{";
        tcl_retval += rowBuf;
        tcl_retval += "}}";
    }
    Tcl_SetResult(interp, (char *)tcl_retval.c_str(), TCL_VOLATILE);
    return TCL_OK;
}

int
tcltdsDisconnect(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]) {
    //fprintf(stdout, "entered disconnect");
    DBConnection *tcltdsDBConn = tcltdsGetConn();
    if (tcltdsDBConn == NULL) {
        Tcl_SetResult(interp, (char*) "ERROR: Invalid connection id pased to tds_disconnect", TCL_VOLATILE);
        return TCL_ERROR;
    }
    if (objc == 1) { // 0 parameters passed, use last connection
        delete tcltdsDBConn;
        tcltdsConnVector.pop_back();
    }
    else if (objc == 2) {
        tcltdsDBConn = tcltdsGetConn(atoi((char *)objv[1]));
        delete tcltdsDBConn;
        vector<tcltdsConnObj>::iterator x = tcltdsConnVector.begin();
        while (x != tcltdsConnVector.end()) {
            if ((*x).connID == atoi((char *)objv[1])) {
                tcltdsConnVector.erase(x);
                break;
            }
            x++;
        }
        //tcltdsConnVector.erase(atoi((char *)objv[1]));
    }
    //fprintf(stdout, "leaving disconnect");
    return TCL_OK;
}


/*DBConnection *
tcltdsTempConn(login_info *login) {
    DBConnection *retVal = NULL;
    try {
        retVal = DBConnection::buildConnection(SQLServer, login->servAddr, login->uid, 
                login->pwd, atoi(login->dbPort.c_str()));
        retVal->connect(login->dbName.c_str());
    }
    catch (DBLibException &e) {
        fprintf(stderr, e.getErrInfo().c_str());
    }
    return retVal;
}*/
