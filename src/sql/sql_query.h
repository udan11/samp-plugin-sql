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

#include <vector>

#include "../sdk/amx/amx.h"

#define QUERY_ERROR_CALLBACK				"OnSQLError"

#define QUERY_THREADED						1
#define QUERY_CACHED						2

#define QUERY_STATUS_NONE					0
#define QUERY_STATUS_EXECUTED				1
#define QUERY_STATUS_PROCESSED				2

class SQL_Query {
	public:
		SQL_Query();
		virtual ~SQL_Query();
		AMX *amx;
		int id, handler, flags, status, error;
		char *query, *callback, *format;
		const char *error_msg;
		std::vector<std::pair<cell*, int> > params_a;
		std::vector<cell> params_c;
		std::vector<char*> params_s;
		unsigned long *last_row_lengths;
		int last_row_idx, insert_id, affected_rows, num_rows, num_fields;
		std::vector<std::pair<char*, int > > field_names;
		std::vector<std::vector<std::pair<char*, int> > > cache;
		int execute_callback();
};