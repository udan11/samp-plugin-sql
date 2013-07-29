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

#include "SQL_Statement.h"

SQL_Statement::SQL_Statement(int id, AMX *amx, int connectionId) {
	this->id = id;
	this->amx = amx;
	this->connectionId = connectionId;
	flags = STATEMENT_FLAGS_NONE;
	status = STATEMENT_STATUS_NONE;
	lastResultIdx = 0;
	query = NULL;
	callback = NULL;
	format = NULL;
	error = 0;
	errorMsg = NULL;
}

SQL_Statement::~SQL_Statement() {
	free(query);
	free(callback);
	free(format);
	for (int i = 0, size = paramsArr.size(); i != size; ++i) {
		free(paramsArr[i].first);
	}
	for (int i = 0, size = paramsStr.size(); i != size; ++i) {
		free(paramsStr[i]);
	}
	for (int i = 0, size = resultSets.size(); i != size; ++i) {
		delete resultSets[i];
	}
}

int SQL_Statement::executeCallback() {
	cell ret, amx_addr = -1;
	int funcidx;
	if (error == 0) {
		if (!amx_FindPublic(amx, callback, &funcidx)) {
			int a_idx = paramsArr.size(), c_idx = paramsC.size(), s_idx = paramsStr.size();
			for (int i = strlen(format) - 1; i != -1; --i) {
				if ((i > 0) && (format[i - 1] == '&')) {
					amx_Push(amx, paramsC[--c_idx]);
					--i; // Skipping next specifier (&x).
				} else {
					cell tmp;
					switch (format[i]) {
						case 'a':
						case 'A':
							amx_PushArray(amx, &tmp, NULL, paramsArr[--a_idx].first, paramsArr[a_idx].second);
							if (amx_addr == -1) {
								amx_addr = tmp;
							}
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
							amx_Push(amx, paramsC[--c_idx]);
							break;
						case 'r':
						case 'R':
							amx_Push(amx, id);
							break;
						case 's':
						case 'S':
							amx_PushString(amx, &tmp, NULL, paramsStr[--s_idx], 0, 0);
							if (amx_addr == -1) {
								amx_addr = tmp;
							}
							break;
					}
				}
			}
			amx_Exec(amx, &ret, funcidx);
		}
	} else {
		if (!amx_FindPublic(amx, ERROR_CALLBACK, &funcidx)) {
			cell tmp;
			amx_PushString(amx, &amx_addr, NULL, callback, 0, 0);
			amx_PushString(amx, &tmp, NULL, query, 0, 0);
			amx_PushString(amx, &tmp, NULL, errorMsg, 0, 0);
			amx_Push(amx, error);
			amx_Push(amx, connectionId);
			amx_Exec(amx, &ret, funcidx);
		}
	}
	if (amx_addr != -1) {
		amx_Release(amx, amx_addr);
	}
	return ret;
}
