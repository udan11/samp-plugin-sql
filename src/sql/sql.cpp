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

#include "../log.h"

#include "sql_handler.h"
#include "sql_query.h"

#include "../mysql/mysql.h"
#include "../pgsql/pgsql.h"

#include "sql.h"

int lastHandler = 1;
handlers_t handlers;

int lastQuery = 1;
queries_t queries;

bool is_valid_handler(int id) {
	return handlers.find(id) != handlers.end();
}

bool is_valid_query(int id) {
	return queries.find(id) != queries.end();
}

#ifdef _WIN32
DWORD WINAPI worker(LPVOID param) {
#else
void *worker(void *param) {
#endif
	SQL_Handler* handler = (SQL_Handler*) param;
#if defined SQL_HANDLER_MYSQL
	if (handler->type == SQL_HANDLER_MYSQL) {
		mysql_thread_init();
	}
#endif
	while (handler->isActive) {
		SQL_Query *query = NULL;
		while (handler->pending.pop(query)) {
			log(LOG_DEBUG, "worker(handlers[%d]): Executing query (query->id = %d, query->query = %s)...", handler->id, query->id, query->query);
			handler->execute_query(query);
		}
		SLEEP(50);
	}
#if defined SQL_HANDLER_MYSQL
	if (handler->type == SQL_HANDLER_MYSQL) {
		mysql_thread_end();
	}
#endif
	return 0;
}
