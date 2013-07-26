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

const AMX_NATIVE_INFO NATIVES[] = {
	{"sql_debug", Natives::sql_debug},
	{"sql_connect", Natives::sql_connect},
	{"sql_disconnect", Natives::sql_disconnect},
	{"sql_wait", Natives::sql_wait},
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
	{"sql_fetch_row", Natives::sql_fetch_row},
	// Polymorphic natives.
	{"sql_next_row", Natives::sql_next_row},
	{"sql_get_field", Natives::sql_get_field},
	{"sql_get_field_assoc", Natives::sql_get_field_assoc},
	{"sql_get_field_int", Natives::sql_get_field_int},
	{"sql_get_field_assoc_int", Natives::sql_get_field_assoc_int},
	{"sql_get_field_float", Natives::sql_get_field_float},
	{"sql_get_field_assoc_float", Natives::sql_get_field_assoc_float},
	// The extended version (includes a `row` parameter).
	{"sql_get_field_ex", Natives::sql_get_field},
	{"sql_get_field_assoc_ex", Natives::sql_get_field_assoc},
	{"sql_get_field_int_ex", Natives::sql_get_field_int},
	{"sql_get_field_assoc_int_ex", Natives::sql_get_field_assoc_int},
	{"sql_get_field_float_ex", Natives::sql_get_field_float},
	{"sql_get_field_assoc_float_ex", Natives::sql_get_field_assoc_float},
	{NULL, NULL}
};

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
	logprintf = (logprintf_t) ppData[PLUGIN_DATA_LOGPRINTF];
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
#ifdef SQL_HANDLER_MYSQL
	if (mysql_library_init(0, NULL, NULL)) {
		logprintf("  >> Coudln't initalize the MySQL library (libmysql.dll). It's probably missing.");
		exit(0);
		return false;
	}
#endif
	logprintf("  >> SQL plugin " PLUGIN_VERSION " successfully loaded.");
#ifdef SQL_HANDLER_MYSQL
	logprintf("      + MySQL support is enabled.");
#endif
#ifdef SQL_HANDLER_PGSQL
	logprintf("      + PostgreSQL support is enabled.");
#endif
	return true;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) {
	return amx_Register(amx, NATIVES, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) {
	for (handlers_t::iterator it = handlers.begin(), next = it, end = handlers.end(); it != end; it = next) {
		++next;
		SQL_Handler *handler = it->second;
		if (handler->amx == amx) {
			handlers.erase(it);
			handler->stop_worker();
			delete handler;
		}
	}
	for (queries_t::iterator it = queries.begin(), next = it, end = queries.end(); it != end; it = next) {
		++next;
		SQL_Query *query = it->second;
		if (query->amx == amx) {
			queries.erase(it);
			delete query;
		}
	}
	return AMX_ERR_NONE;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
#ifdef SQL_HANDLER_MYSQL
	mysql_library_end();
#endif
	logprintf("[plugin.sql] Plugin succesfully unloaded!");
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() {
	for (queries_t::iterator it = queries.begin(), next = it, end = queries.end(); it != end; it = next) {
		++next;
		SQL_Query *query = it->second;
		if ((query->flags & QUERY_THREADED) && (query->status == QUERY_STATUS_EXECUTED)) {
			log(LOG_DEBUG, "ProccessTick: Executing query callback (query->id = %d, query->error = %d, query->callback = %s)...", query->id, query->error, query->callback);
			query->status = QUERY_STATUS_PROCESSED;
			int id = query->id;
			query->execute_callback();
			if (!is_valid_query(id)) {
				continue;
			}
		}
		if ((!is_valid_handler(query->handler)) || (query->status == QUERY_STATUS_PROCESSED)) {
			log(LOG_DEBUG, "ProccessTick: Erasing query (query->id = %d)...", query->id);
			queries.erase(it);
			delete query;
		}
	}
}
