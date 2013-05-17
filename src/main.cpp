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

#include "main.h"

extern void *pAMXFunctions;

Mutex *amxMutex;

int last_handler = 1, last_query = 1;
std::map<int, class SQL_Handler*> handlers;
std::map<int, class SQL_Query*> queries;

#ifdef _WIN32
DWORD __stdcall ProcessQueryThread(LPVOID lpParam);
#else
void *ProcessQueryThread(void *lpParam);
#endif

const AMX_NATIVE_INFO NATIVES[] = {
	{"sql_debug", Natives::sql_debug},
	{"sql_connect", Natives::sql_connect},
	{"sql_disconnect", Natives::sql_disconnect},
	{"sql_set_charset", Natives::sql_set_charset},
	{"sql_get_charset", Natives::sql_get_charset},
	{"sql_ping", Natives::sql_ping},
	{"sql_get_stat", Natives::sql_get_stat},
	{"sql_escape_string", Natives::sql_escape_string},
	{"sql_query", Natives::sql_query},
	{"sql_store_result", Natives::sql_store_result},
	{"sql_free_result", Natives::sql_free_result},
	{"sql_insert_id", Natives::sql_insert_id},
	{"sql_affected_rows", Natives::sql_affected_rows},
	{"sql_error", Natives::sql_error},
	{"sql_error_string", Natives::sql_error_string},
	{"sql_num_rows", Natives::sql_num_rows},
	{"sql_num_fields", Natives::sql_num_fields},
	{"sql_next_result", Natives::sql_next_result},
	{"sql_field_name", Natives::sql_field_name},
	{"sql_next_row", Natives::sql_next_row},
	{"sql_get_field", Natives::sql_get_field},
	{"sql_get_field_assoc", Natives::sql_get_field_assoc},
	{"sql_get_field_int", Natives::sql_get_field_int},
	{"sql_get_field_assoc_int", Natives::sql_get_field_assoc_int},
	{"sql_get_field_float", Natives::sql_get_field_float},
	{"sql_get_field_assoc_float", Natives::sql_get_field_assoc_float},
	{0, 0}
};

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
	logprintf = (logprintf_t) ppData[PLUGIN_DATA_LOGPRINTF];
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
#ifdef SQL_HANDLER_MYSQL
	if (mysql_library_init(0, 0, 0)) {
		logprintf("  >> Coudln't initalize the MySQL library (libmysql.dll). It's probably missing.");
		exit(0);
		return 0;
	}
#endif
	amxMutex = new Mutex();
#ifdef _WIN32
	HANDLE thread;
	DWORD threadId = 0;
	thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) ProcessQueryThread, 0, 0, &threadId);
	CloseHandle(thread);
#else
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_t thread;
	pthread_create(&thread, &attr, &ProcessQueryThread, 0);
#endif
	logprintf("  >> MySQL plugin " PLUGIN_VERSION " successfully loaded.");
	return true;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) {
	return amx_Register(amx, NATIVES, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) {
	amxMutex->lock();
	for (std::map<int, class SQL_Handler*>::iterator it = handlers.begin(), next = it; it != handlers.end(); it = next) {
		++next;
		SQL_Handler *handler = it->second;
		if (handler->amx == amx) {
			delete handler;
			handlers.erase(it);
		}
	}
	for (std::map<int, class SQL_Query*>::iterator it = queries.begin(), next = it; it != queries.end(); it = next) {
		++next;
		SQL_Query *query = it->second;
		if (query->amx == amx) {
			delete query;
			queries.erase(it);
		}
	}
	amxMutex->unlock();
	return AMX_ERR_NONE;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
	delete amxMutex;
	logprintf("[plugin.mysql] Plugin succesfully unloaded!");
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() {
	amxMutex->lock();
	for (std::map<int, class SQL_Query*>::iterator it = queries.begin(), next = it; it != queries.end(); it = next) {
		++next;
		SQL_Query *query = it->second;
		if ((query->flags & QUERY_THREADED) && (query->status == QUERY_STATUS_EXECUTED)) {
			log(LOG_DEBUG, "ProccessTick(): Executing query callback (query->id = %d, query->error = %d, query->callback = %s)...", query->id, query->error, query->callback);
			query->status = QUERY_STATUS_PROCESSED;
			query->execute_callback();
		}
		if ((!is_valid_handler(query->handler)) || (query->status == QUERY_STATUS_PROCESSED)) {
			log(LOG_DEBUG, "ProccessTick(): Erasing query (query->id = %d)...", query->id);
			delete query;
			queries.erase(it);
		}
	}
	amxMutex->unlock();
}

#ifdef _WIN32
DWORD __stdcall ProcessQueryThread(LPVOID lpParam) {
#else
void *ProcessQueryThread(void *lpParam) {
#endif
	while (true) {
		amxMutex->lock();
		for (std::map<int, class SQL_Query*>::iterator it = queries.begin(), next = it; it != queries.end(); it = next) {
			++next;
			SQL_Query *query = it->second;
			if ((query->flags & QUERY_THREADED) && (query->status == QUERY_STATUS_NONE)) {
				log(LOG_DEBUG, "ProcessQueryThread(): Executing query (query->id = %d, query->query = %s)...", query->id, query->query);
				handlers[query->handler]->execute_query(query);
			}
		}
		amxMutex->unlock();
		SLEEP(50);
	}
	return 0;
}