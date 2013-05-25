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

#include "sql_query.h"

SQL_Query::SQL_Query() {
	amx = 0;
	id = 0;
	handler = 0;
	flags = 0;
	status = 0;
	error = 0;
	last_result = 0;
	query = 0;
	callback = 0;
	format = 0;
	error_msg = 0;
	params_a.clear();
	params_c.clear();
	params_s.clear();
	results.clear();
}

SQL_Query::~SQL_Query() {
	free(query);
	free(callback);
	free(format);
	for (int i = 0, size = params_a.size(); i != size; ++i) {
		free(params_a[i].first);
	}
	params_a.clear();
	params_c.clear();
	for (int i = 0, size = params_s.size(); i != size; ++i) {
		free(params_s[i]);
	}
	params_s.clear();
	for (int i = 0, size = results.size(); i != size; ++i) {
		delete results[i];
	}
	results.clear();
}

int SQL_Query::execute_callback() {
	cell ret, amx_addr = -1;
	int funcidx;
	if (error == 0) {
		if (!amx_FindPublic(amx, callback, &funcidx)) {
			int a_idx = params_a.size(), c_idx = params_c.size(), s_idx = params_s.size();
			for (int i = strlen(format) - 1; i != -1; --i) {
				switch (format[i]) {
					case 'a':
					case 'A':
						if (amx_addr < 0) {
							amx_addr = 0;
						}
						amx_PushArray(amx, &amx_addr, 0, params_a[--a_idx].first, params_a[a_idx].second);
						break;
					case 'b':
					case 'B':
					case 'c':
					case 'C':
					case 'd':
					case 'D':
					case 'i':
					case 'I':
					case 'f':
					case 'F':
						amx_Push(amx, params_c[--c_idx]);
						break;
					case 'r':
					case 'R':
						amx_Push(amx, id);
						break;
					case 's':
					case 'S':
						if (amx_addr < 0) {
							amx_addr = 0;
						}
						amx_PushString(amx, &amx_addr, 0, params_s[--s_idx], 0, 0);
						break;
				}
			}
			amx_Exec(amx, &ret, funcidx);
		}
	} else {
		if (!amx_FindPublic(amx, QUERY_ERROR_CALLBACK, &funcidx)) {
			amx_addr = 0;
			amx_PushString(amx, &amx_addr, 0, callback, 0, 0);
			amx_PushString(amx, &amx_addr, 0, query, 0, 0);
			amx_PushString(amx, &amx_addr, 0, error_msg, 0, 0);
			amx_Push(amx, (cell) error);
			amx_Push(amx, (cell) handler);
			amx_Exec(amx, &ret, funcidx);
		}
	}
	if (amx_addr >= 0) {
		amx_Release(amx, amx_addr);
	}
	return (int) ret;
}