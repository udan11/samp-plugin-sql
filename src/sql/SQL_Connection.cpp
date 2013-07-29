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

#include "SQL_Statement.h"

#if defined PLUGIN_SUPPORTS_MYSQL
	#include "mysql/mysql.h"
#endif

#if defined PLUGIN_SUPPORTS_PGSQL
	#include "pgsql/pgsql.h"
#endif

#include "SQL_Connection.h"

#ifdef _WIN32
DWORD WINAPI SQL_Worker(LPVOID param) {
#else
void *SQL_Worker(void *param) {
#endif
	SQL_Connection* conn = (SQL_Connection*) param;
	#if defined PLUGIN_SUPPORTS_MYSQL
		if (conn->type == PLUGIN_SUPPORTS_MYSQL) {
			mysql_thread_init();
		}
	#endif
	while (conn->isActive) {
		SQL_Statement *stmt = NULL;
		while (conn->pending.pop(stmt)) {
			log(LOG_DEBUG, "sqlWorkerThread[%d]: Executing query (stmt->id = %d, stmt->query = %s)...", conn->id, stmt->id, stmt->query);
			conn->executeStatement(stmt);
		}
		SLEEP(WORKER_TICK_RATE);
	}
	#if defined PLUGIN_SUPPORTS_MYSQL
		if (conn->type == PLUGIN_SUPPORTS_MYSQL) {
			mysql_thread_end();
		}
	#endif
	return 0;
}

SQL_Connection::SQL_Connection(int id, AMX *amx) : pending(32) {
	this->id = id;
	this->amx = amx;
	this->thread = NULL;
}

SQL_Connection::~SQL_Connection() {
	stopWorker();
}

void SQL_Connection::startWorker() {
	if (thread == NULL) {
		isActive = true;
		#ifdef _WIN32
			DWORD threadId = 0;
			thread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE) SQL_Worker, (LPVOID) this, NULL, &threadId);
		#else
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
			pthread_create(&thread, &attr, &SQL_Worker, (void*) this);
			pthread_attr_destroy(&attr);
		#endif
	}
}

void SQL_Connection::stopWorker() {
	if (thread != NULL) {
		isActive = false;
#ifdef _WIN32
		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
#else
		void *status;
		pthread_join(thread, &status);
#endif
		thread = NULL;
	}
}
