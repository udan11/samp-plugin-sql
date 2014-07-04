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

#
# make <flags>
#
# Flag list:
# MYSQL=true  - adds support for MySQL 
# PGSQL=true  - adds support for PostgreSQL
# STATIC=true - links statically MySQL library (only!)
#

ifndef CC
  CC = gcc
endif
ifndef GXX
  GXX = g++
endif

# Compilation flags.
COMPILE_FLAGS = -c -fPIC -m32 -O3 -w -Wall -Iinclude/ -DLINUX
LIBRARIES = -lpthread -lrt

# Output file name.
OUTFILE = bin/sql.so

# 1: Linking MySQL library (if it's necessary).
ifneq ($(MYSQL),)
  COMPILE_FLAGS += -DPLUGIN_SUPPORTS_MYSQL=1
  ifneq ($(STATIC),)
    LIBRARIES += -ldl ./lib/mysql/libmysql.a
    OUTFILE := bin/mysql_static.so
  else
    LIBRARIES += ./lib/mysql/libmysql.so
    OUTFILE := bin/mysql.so
  endif
endif

# 2: Linking PostgreSQL library (if it's necessary).
ifneq ($(PGSQL),)
  COMPILE_FLAGS += -DPLUGIN_SUPPORTS_PGSQL=2
  # There is no way to link statically `libpq`.
  LIBRARIES += ./lib/pgsql/libpq.so
  OUTFILE := bin/pgsql.so
endif

# It has both (or neither) MySQL and PgSQL support.
ifneq ($(MYSQL),)
  ifneq ($(PGSQL),)
    ifneq ($(STATIC),)
      OUTFILE := bin/sql_static.so
    else
      OUTFILE := bin/sql.so
    endif
  endif
endif

# Compiling!
all:
  mkdir -p foo
  $(GXX) $(COMPILE_FLAGS) src/sdk/*.cpp
  $(GXX) $(COMPILE_FLAGS) src/sql/*.cpp
  $(GXX) $(COMPILE_FLAGS) src/sql/mysql/*.cpp
  $(GXX) $(COMPILE_FLAGS) src/sql/pgsql/*.cpp
  $(GXX) $(COMPILE_FLAGS) src/*.cpp
  $(GXX) -m32 -shared -o $(OUTFILE) *.o $(LIBRARIES) 
  
clean:
  rm -f *.o
