# Copyright (c) 2013, Dan
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met: 
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer. 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution. 
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

GCC = gcc
GPP = g++

# Compilation flags.
COMPILE_FLAGS = -c -fPIC -m32 -O3 -w -Wall -Iinclude/ -DLINUX
LIBRARIES = -lpthread

# Output filename.
OUTFILE = bin/sql.so

# 1: Linking MySQL library (if it's necessary).
ifdef MYSQL
	COMPILE_FLAGS += -DSQL_HANDLER_MYSQL=1
	LIBRARIES += ./lib/mysql/libmysql.so
	OUTFILE := bin/sql_mysql.so
endif

# 2: Linking PostgreSQL library (if it's necessary).
ifdef PGSQL
	COMPILE_FLAGS += -DSQL_HANDLER_PGSQL=2
	LIBRARIES += ./lib/pgsql/libpq.so
	OUTFILE := bin/sql_pgsql.so
endif

# It has both MySQL & PgSQL support.
ifdef MYSQL
	ifdef PGSQL
		OUTFILE := bin/sql.so
	endif
else
	ifndef PGSQL
		ECHO "Error! MYSQL & PGSQL weren't defined."
	endif
endif

# Compiling!
all:
	$(GPP) $(COMPILE_FLAGS) src/sdk/*.cpp
	$(GPP) $(COMPILE_FLAGS) src/sql/*.cpp
	$(GPP) $(COMPILE_FLAGS) src/mysql/*.cpp
	$(GPP) $(COMPILE_FLAGS) src/pgsql/*.cpp
	$(GPP) $(COMPILE_FLAGS) src/*.cpp
	$(GPP) -fshort-wchar -shared -o $(OUTFILE) *.o $(LIBRARIES) 
	rm -f *.o
