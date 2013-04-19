/**
 * SA:MP Plugin - MySQL
 * Copyright (C) 2013 Dan
 *  
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <map>
#include <vector>

#if ((defined(WIN32)) || (defined(_WIN32)) || (defined(_WIN64)))
	#include "windows.h"
	#define SLEEP(x) Sleep(x);
#else
	#include "pthread.h"
	#include <unistd.h>
	#define SLEEP(x) usleep(x * 1000);
	typedef unsigned long DWORD;
	typedef unsigned int UINT;
#endif

#include "mysql_include/mysql.h"

#include "SDK/amx/amx.h"
#include "SDK/plugincommon.h"
#include "SDK/string.h"

#include "log.h"
#include "mutex.h"
#include "mysql_handler.h"
#include "mysql_query.h"
#include "mysql_utils.h"
#include "natives.h"

#define PLUGIN_VERSION					"v0.7 BETA"
#define QUERY_ERROR_CALLBACK			"OnMySQLError"

extern int last_handler, last_query;
extern std::map<int, class MySQL_Handler*> handlers;
extern std::map<int, struct mysql_query*> queries;