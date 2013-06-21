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

//#define SQL_HANDLER_MYSQL				1
//#define SQL_HANDLER_PGSQL				2

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

#include "sql_handler.h"
#include "sql_query.h"
#include "sql_result.h"

#define TICK_RATE						50

extern int lastHandler;
extern handlers_t handlers;

extern int lastQuery;
extern queries_t queries;

extern bool is_valid_handler(int id);
extern bool is_valid_query(int id);

#ifdef _WIN32
	extern DWORD WINAPI worker(LPVOID param);
#else
	extern void *worker(void *param);
#endif
