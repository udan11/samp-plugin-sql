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

#include <boost/unordered/unordered_map.hpp>

#ifdef _WIN32
	#include <Windows.h>
#else
	#include "pthread.h"
	#include <unistd.h>
#endif

#include "../sdk/amx/amx.h"

#include "sql_query.h"

class SQL_Handler {
	public:
		int id, type;
		AMX *amx;
		query_qt pending;
		bool isActive;
	#ifdef _WIN32
		HANDLE thread;
	#else
		pthread_t thread;
	#endif
		SQL_Handler(int id, AMX *amx);
		~SQL_Handler();
		virtual bool connect(const char *host, const char *user, const char *pass, const char *db, int port) = 0;
		virtual void disconnect() = 0;
		virtual int get_errno() = 0;
		virtual const char *get_error() = 0;
		virtual int ping() = 0;
		virtual const char *get_stat() = 0;
		virtual const char *get_charset() = 0;
		virtual bool set_charset(char *charset) = 0;
		virtual int escape_string(const char *src, char *&dest) = 0;
		virtual void execute_query(SQL_Query *query) = 0;
		virtual bool seek_result(SQL_Query *query, int result) = 0;
		virtual bool fetch_field(SQL_Query *query, int fieldix, char *&dest, int &len) = 0;
		virtual bool seek_row(SQL_Query *query, int row) = 0;
		virtual bool fetch_num(SQL_Query *query, int fieldidx, char *&dest, int &len) = 0;
		virtual bool fetch_assoc(SQL_Query *query, char *fieldname, char *&dest, int &len) = 0;
};

typedef boost::unordered_map<int, class SQL_Handler*> handlers_t;