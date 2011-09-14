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

#include "tcl_hooks.h"

extern "C" {
    int
    Tcltds_Init(Tcl_Interp * interp)
    {
	if (Tcl_InitStubs(interp, "8.4", 0) == NULL) {
		return TCL_ERROR;
	}
        Tcl_CreateCommand(interp, "tds_connect", (Tcl_CmdProc *) tcltdsConnect, NULL, NULL);
        Tcl_CreateCommand(interp, "tds_query", (Tcl_CmdProc *) tcltdsQuery, NULL, NULL);
        Tcl_CreateCommand(interp, "tds_fetchrow", (Tcl_CmdProc *) tcltdsFetchRow, NULL, NULL);
        Tcl_CreateCommand(interp, "tds_fetchset", (Tcl_CmdProc *) tcltdsFetchSet, NULL, NULL);
        Tcl_CreateCommand(interp, "tds_disconnect", (Tcl_CmdProc *) tcltdsDisconnect, NULL, 
                NULL);
	Tcl_PkgProvide(interp, "tcltds", "1.0");

		return TCL_OK;
        return TCL_OK;
    }
}
