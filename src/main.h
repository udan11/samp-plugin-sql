/**
 * Copyright (c) 2013, Dan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <cstdio>
#include <cstdlib>

#include <map>
#include <vector>

#ifdef _WIN32
	#include <Windows.h>
	#define SLEEP(x) Sleep(x);
#else
	#include "pthread.h"
	#include <unistd.h>
	#define SLEEP(x) usleep(x * 1000);
	typedef unsigned long DWORD;
	typedef unsigned int UINT;
#endif

#include "sdk/amx/amx.h"
#include "sdk/plugincommon.h"
#include "sdk/amxstring.h"

#include "sql/sql_handler.h"
#include "sql/sql_query.h"
#include "sql/sql_result.h"
#include "sql/sql_utils.h"

#ifdef SQL_HANDLER_MYSQL
	#include "mysql/mysql.h"
#endif

#ifdef SQL_HANDLER_PGSQL
	#include "pgsql/pgsql.h"
#endif

#include "log.h"
#include "mutex.h"
#include "natives.h"

#define PLUGIN_VERSION					"v2.0"

extern Mutex *amxMutex;

extern int last_handler, last_query;
extern std::map<int, class SQL_Handler*> handlers;
extern std::map<int, class SQL_Query*> queries;