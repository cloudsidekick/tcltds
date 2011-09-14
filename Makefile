#########################################################################
# Copyright 2011 Cloud Sidekick
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#########################################################################

########################################
#  CONFIGURATION VARIABLES
#
# Change the following lines to match
# tell the compiler where the tcl
# files are and which version
#
########################################
TDS_INCLUDE_HOME=/usr/include
TDS_LIB_HOME=/usr/lib
TCL_INCLUDE_HOME=/usr/include/tcl8.5
TCL_LIB_HOME=/usr/lib
TCL_LIB_VERSION=8.5

########################################
#  You should not need to edit below
#  this line
########################################
GCC = `which g++`
LIBSRC = utils.cc tcl_hooks.cc dbconnection.cc sqlserverconnection.cc tcltds.cc
LIB_SO = lib/libtcltds.so
PROJ_NAME = "Tcl TDS Library"

########################################
#  VARIABLE POST-PROCESSING                                                 
########################################
LIBSRC_C = $(LIBSRC:%.cc=src/%.cc)
LIBSRC_O = $(LIBSRC:%.cc=obj/%.o)

########################################
#  TCLTDS LIB BUILD                               
########################################
libtcltds: CFLAGS=-g -I./include -I$(TCL_INCLUDE_HOME) -I$(TDS_INCLUDE_HOME) -fPIC
libtcltds: LDFLAGS= -L$(TCL_LIB_HOME)/lib -L$(TDS_LIB_HOME)/lib -ltcl$(TCL_LIB_VERSION) -lsybdb
libtcltds: PHASE="Tcl TDS Library"
libtcltds: sep $(LIBSRC_O)
	@echo $(GCC) $(LDFLAGS) -shared -o $(LIB_SO) $(LIBSRC_O)
	@$(GCC) $(LDFLAGS) -shared -o $(LIB_SO) $(LIBSRC_O)
	
########################################
#  SEPARATOR                                                                
########################################
sep:
	@echo -------------------------------------------------------
	@echo Building $(PROJ_NAME) \( $(PHASE) \)

########################################
#  DESTROY BINARIES                                               
########################################

clean:
	@echo "Removing binaries..."
	@echo rm -f $(LIBSRC_O) $(LIB_SO)
	@rm -f $(LIBSRC_O) $(LIB_SO)

########################################
#  BINARY DEPENDANCIES                                             
########################################
obj/%.o: src/%.cc
	@printf "[33m :: compiling %-20s[0m | [36m%s[0m\n" $(<F) $(PHASE)
	@echo $(GCC) $(CFLAGS) -c $< -o $@
	@$(GCC) $(CFLAGS) -c $< -o $@
