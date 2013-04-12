#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <map>
#include <vector>

#if ((defined(WIN32)) || (defined(_WIN32)) || (defined(_WIN64)))
	#include "windows.h"
#else
	#include "pthread.h"
#endif

#include "SDK/amx/amx.h"
#include "SDK/plugincommon.h"
#include "SDK/string.h"

#include "mysql_include/mysql.h"

#include "log.h"
#include "mutex.h"
#include "mysql_handler.h"
#include "mysql_query.h"
#include "mysql_utils.h"
#include "natives.h"

#ifdef WIN32
	#define SLEEP(x) Sleep(x);
#else
	#include <unistd.h>
	#define SLEEP(x) usleep(x * 1000);
	typedef unsigned long DWORD;
	typedef unsigned int UINT;
#endif

#define PLUGIN_VERSION					"v0.3 BETA"
#define PLUGIN_AUTHOR					"Dan"

#define QUERY_ERROR_CALLBACK			"OnMySQLError"

extern int last_handler, last_query;
extern std::map<int, class MySQL_Handler*> handlers;
extern std::map<int, struct mysql_query*> queries;