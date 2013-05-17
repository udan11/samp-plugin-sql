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

#include "natives.h"

cell AMX_NATIVE_CALL Natives::sql_debug(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	amxMutex->lock();
	log_level_file = params[1];
	log_level_console = params[2];
	log(LOG_WARNING, "Natives::sql_debug: Switching the log levels to (%d, %d)...", log_level_file, log_level_console);
	amxMutex->unlock();
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_connect(AMX *amx, cell *params) {
	if (params[0] < 6 * 4) {
		return 0;
	}
	amxMutex->lock();
	SQL_Handler *handler;
	switch (params[1]) {
#ifdef SQL_HANDLER_MYSQL
		case SQL_HANDLER_MYSQL:
			handler = new MySQL_Handler();
			break;
#endif
#ifdef SQL_HANDLER_PGSQL
		case SQL_HANDLER_PGSQL:
			handler = new PgSQL_Handler();
			break;
#endif
		default:
			log(LOG_INFO, "Natives::sql_connect: Unknown SQL type (%d)!", params[1]);
			amxMutex->unlock();
			return 0;
	}
	int id = last_handler++;
	handler->id = id;
	handler->handler_type = params[1];
	handler->amx = amx;
	char *host = 0, *user = 0, *pass = 0, *db = 0;
	amx_GetString_(amx, params[2], host);
	amx_GetString_(amx, params[3], user);
	amx_GetString_(amx, params[4], pass);
	amx_GetString_(amx, params[5], db);
	int port = params[6];
	if (port == 0) {
		switch (params[1]) {
#ifdef SQL_HANDLER_MYSQL
			case SQL_HANDLER_MYSQL:
				port = MYSQL_DEFAULT_PORT;
				break;
#endif
#ifdef SQL_HANDLER_PGSQL
			case SQL_HANDLER_PGSQL:
				port = PGSQL_DEFAULT_PORT;
				break;
#endif
		}
	}
	log(LOG_INFO, "Natives::sql_connect: Connecting to %s:***@%s:%d/%s...", user, host, port, db);
	bool connected = handler->connect(host, user, pass, db, port);
	free(host);
	free(user);
	free(pass);
	free(db);
	if (connected) {
		log(LOG_INFO, "Natives::sql_connect: Connection was succesful!");
		handlers[id] = handler;
		amxMutex->unlock();
		return id;
	}
	log(LOG_INFO, "Natives::sql_connect: Connection failed! (error = %d, %s)", handler->get_errno(), handler->get_error());
	delete handler;
	amxMutex->unlock();
	return 0;
}

cell AMX_NATIVE_CALL Natives::sql_disconnect(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	amxMutex->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		amxMutex->unlock();
		return 0;
	}
	delete handlers[handler_id];
	handlers.erase(handler_id);
	log(LOG_INFO, "Natives::sql_disconnect: Handler %d was destroyed!", handler_id);
	amxMutex->unlock();
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_set_charset(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	amxMutex->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		amxMutex->unlock();
		return 0;
	}
	char *charset = 0;
	amx_GetString_(amx, params[2], charset);
	bool ret = handlers[handler_id]->set_charset(charset);
	log(LOG_INFO, "Natives::sql_set_charset: Charset %s was set (%d)!", charset, ret);
	free(charset);
	amxMutex->unlock();
	return ret;
}

cell AMX_NATIVE_CALL Natives::sql_get_charset(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	amxMutex->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		amxMutex->unlock();
		return 0;
	}
	char *tmp = (char*) handlers[handler_id]->get_charset();
	log(LOG_DEBUG, "Natives::sql_get_charset: charset = %s", tmp);
	int dest_len = params[3], len = strlen(tmp);
	if (dest_len < 2) {
		amx_SetString_(amx, params[2], tmp, len);
	} else {
		amx_SetString_(amx, params[2], tmp, dest_len);
	}
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_ping(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return -1;
	}
	amxMutex->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		amxMutex->unlock();
		return -1;
	}
	int ping = handlers[handler_id]->ping();
	log(LOG_DEBUG, "Natives::sql_ping: Pinging handler %d, recieved result %d.", handler_id, ping);
	amxMutex->unlock();
	return ping;
}

cell AMX_NATIVE_CALL Natives::sql_get_stat(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	amxMutex->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		amxMutex->unlock();
		return 0;
	}
	char *tmp = (char*) handlers[handler_id]->get_stat();
	log(LOG_DEBUG, "Natives::sql_get_stat: stat = %s", tmp);
	int dest_len = params[3], len = strlen(tmp);
	if (dest_len < 2) {
		amx_SetString_(amx, params[2], tmp, len);
	} else {
		amx_SetString_(amx, params[2], tmp, dest_len);
	}
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_escape_string(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return -1;
	}
	amxMutex->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		amxMutex->unlock();
		return -1;
	}
	char *src = 0;
	amx_GetString_(amx, params[2], src);
	char *dest = (char*) malloc(sizeof(char) * strlen(src) * 2); // *2 in case every character is escaped
	int dest_len = params[4], len = handlers[handler_id]->escape_string(src, dest);
	free(src);
	if (len != 0) {
		if (dest_len < 2) {
			amx_SetString_(amx, params[3], dest, len);
		} else {
			amx_SetString_(amx, params[3], dest, dest_len);
		}
		free(dest);
	}
	amxMutex->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_query(AMX *amx, cell *params) {
	if (params[0] < 5 * 4) {
		return 0;
	}
	amxMutex->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query;
	switch (handlers[handler_id]->handler_type) {
#ifdef SQL_HANDLER_MYSQL
		case SQL_HANDLER_MYSQL:
			query = new MySQL_Query();
			break;
#endif
#ifdef SQL_HANDLER_PGSQL
		case SQL_HANDLER_PGSQL:
			query = new PgSQL_Query();
			break;
#endif SQL_HANDLER_PGSQL
		default:
			amxMutex->unlock();
			return 0;
	}
	int id = last_query++;
	query->amx = amx;
	query->id = id;
	query->handler = handler_id;
	amx_GetString_(amx, params[2], query->query);
	query->flags = params[3];
	query->status = 0;
	amx_GetString_(amx, params[4], query->callback);
	amx_GetString_(amx, params[5], query->format);
	for (int i = 0, len = strlen(query->format), p = 6; i != len; ++i, ++p) {
		switch (query->format[i]) {
			case 'a':
			case 'A':
				cell *ptr_arr, *ptr_len, *arr, len;
				amx_GetAddr(amx, params[p], &ptr_arr);
				amx_GetAddr(amx, params[p + 1], &ptr_len);
				len = sizeof(cell) * (*ptr_len);
				arr = (cell*) malloc(len);
				if (arr != 0) {
					memcpy(arr, ptr_arr, len);
					query->params_a.push_back(std::make_pair(arr, *ptr_len));
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
				cell *ptr;
				amx_GetAddr(amx, params[p], &ptr);
				query->params_c.push_back(*ptr);
				break;
			case 'r':
			case 'R':
				--p; // We didn't read any parameter.
				break;
			case 's':
			case 'S':
				char *str;
				amx_GetString_(amx, params[p], str);
				query->params_s.push_back(str);
				break;
			default: 
				log(LOG_WARNING, "Format '%c' is not recognized.", query->format[i]);
				break;
		}
	}
	log(LOG_DEBUG, "Natives::sql_query: Scheduling query %d: \"%s\", callback: %s(%s) for execution...", query->id, query->query, query->callback, query->format);
	queries[query->id] = query;
	if (!(query->flags & QUERY_THREADED)) {
		log(LOG_DEBUG, "Natives::sql_query: Executing query->id = %d...", query->id);
		handlers[query->handler]->execute_query(query);
		if (strlen(query->callback)) {
			log(LOG_DEBUG, "Natives::sql_query: Executing query callback (query->error = %d)...", query->error);
			query->execute_callback();
		}
		amxMutex->unlock();
		return id;
	}
	amxMutex->unlock();
	return id;
}

cell AMX_NATIVE_CALL Natives::sql_free_result(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	log(LOG_DEBUG, "Natives::sql_query: Freeing query %d...", query->id);
	delete query;
	queries.erase(query_id);
	amxMutex->unlock();
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_store_result(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	log(LOG_DEBUG, "Natives::sql_query: Storing query %d...", query->id);
	query->flags &= ~QUERY_THREADED;
	query->status = QUERY_STATUS_EXECUTED;
	amxMutex->unlock();
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_insert_id(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	int insert_id = query->results[query->last_result]->insert_id;
	amxMutex->unlock();
	return insert_id;
}

cell AMX_NATIVE_CALL Natives::sql_affected_rows(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	int affected_rows = query->results[query->last_result]->affected_rows;
	amxMutex->unlock();
	return affected_rows;
}

cell AMX_NATIVE_CALL Natives::sql_error(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	int error = queries[query_id]->error;
	amxMutex->unlock();
	return error;
}

cell AMX_NATIVE_CALL Natives::sql_error_string(AMX *amx, cell *params) {
	if (params[0] < 3 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		amxMutex->unlock();
		return 0;
	}
	int dest_len = params[3], len = strlen(query->error_msg);
	if (len != 0) {
		char *error = (char*) malloc(sizeof(char) * len);
		strcpy(error, query->error_msg);
		if (dest_len < 2) {
			amx_SetString_(amx, params[2], error, len);
		} else {
			amx_SetString_(amx, params[2], error, dest_len);
		}
	}
	amxMutex->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_num_rows(AMX *amx, cell *params) {
	if (params[0] < 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	int num_rows = query->results[query->last_result]->num_rows;
	amxMutex->unlock();
	return num_rows;
}

cell AMX_NATIVE_CALL Natives::sql_num_fields(AMX *amx, cell *params) {
	if (params[0] < 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	int num_fields = query->results[query->last_result]->num_fields;
	amxMutex->unlock();
	return num_fields;
}

cell AMX_NATIVE_CALL Natives::sql_next_result(AMX *amx, cell* params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		amxMutex->unlock();
		return 0;
	}
	int ret = handlers[query->handler]->seek_result(query, params[2]);
	amxMutex->unlock();
	return ret;
}

cell AMX_NATIVE_CALL Natives::sql_field_name(AMX *amx, cell *params) {
	if (params[0] < 4 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		amxMutex->unlock();
		return 0;
	}
	int dest_len = params[4], len;
	char *tmp = 0;
	bool isCopy = handlers[query->handler]->fetch_field(query, params[2], tmp, len);
	log(LOG_DEBUG, "Natives::sql_field_name: dest_len = %d, len = %d, tmp = %s", dest_len, len, tmp);
	if (len != 0) {
		if (dest_len < 2) {
			amx_SetString_(amx, params[3], tmp, len);
		} else {
			amx_SetString_(amx, params[3], tmp, dest_len);
		}
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_field_name: Can't find field %d or result is empty.", params[2]);
	}
	amxMutex->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_next_row(AMX *amx, cell* params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		amxMutex->unlock();
		return 0;
	}
	int ret = handlers[query->handler]->seek_row(query, params[2]);
	amxMutex->unlock();
	return ret;
}

cell AMX_NATIVE_CALL Natives::sql_get_field(AMX *amx, cell *params) {
	if (params[0] < 4 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		amxMutex->unlock();
		return 0;
	}
	int fieldidx = params[2], dest_len = params[4], len;
	char *tmp = 0;
	bool isCopy = handlers[query->handler]->fetch_num(query, fieldidx, tmp, len);
	if (len != 0) {
		if (dest_len < 2) { // Probably a multi-dimensional array.
			amx_SetString_(amx, params[3], tmp, len);
		} else {
			amx_SetString_(amx, params[3], tmp, dest_len);
		}
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field: Can't find field %d or result is empty.", fieldidx);
	}
	amxMutex->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_assoc(AMX *amx, cell *params) {
	if (params[0] < 4 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		amxMutex->unlock();
		return 0;
	}
	char *fieldname = 0, *tmp = 0;
	amx_GetString_(amx, params[2], fieldname);
	int dest_len = params[4], len;
	bool isCopy = handlers[query->handler]->fetch_assoc(query, fieldname, tmp, len);
	if (len != 0) {
		if (dest_len < 2) {
			amx_SetString_(amx, params[3], tmp, len);
		} else {
			amx_SetString_(amx, params[3], tmp, dest_len);
		}
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field_assoc: Can't find field %s or result is empty.", fieldname);
	}
	free(fieldname);
	amxMutex->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_int(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		amxMutex->unlock();
		return 0;
	}
	int fieldidx = params[2], len, val = 0;
	char *tmp = 0;
	bool isCopy = handlers[query->handler]->fetch_num(query, fieldidx, tmp, len);
	if (len != 0) {
		val = atoi(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field_int: Can't find field %d or result is empty.", params[2]);
	}
	amxMutex->unlock();
	return val;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_assoc_int(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		amxMutex->unlock();
		return 0;
	}
	char *fieldname = 0, *tmp = 0;
	amx_GetString_(amx, params[2], fieldname);
	int len, val = 0;
	bool isCopy = handlers[query->handler]->fetch_assoc(query, fieldname, tmp, len);
	if (len != 0) {
		val = atoi(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field_assoc_int: Can't find field %s or result is empty.", fieldname);
	}
	free(fieldname);
	amxMutex->unlock();
	return val;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_float(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		amxMutex->unlock();
		return 0;
	}
	int fieldidx = params[2], len;
	char *tmp = 0;
	bool isCopy = handlers[query->handler]->fetch_num(query, fieldidx, tmp, len);
	float val = 0.0;
	if (len != 0) {
		val = (float) atof(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field_int: Can't find field %d or result is empty.", params[2]);
	}
	amxMutex->unlock();
	return amx_ftoc(val);
}

cell AMX_NATIVE_CALL Natives::sql_get_field_assoc_float(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	amxMutex->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		amxMutex->unlock();
		return 0;
	}
	SQL_Query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		amxMutex->unlock();
		return 0;
	}
	char *fieldname = 0, *tmp = 0;
	amx_GetString_(amx, params[2], fieldname);
	int len;
	bool isCopy = handlers[query->handler]->fetch_assoc(query, fieldname, tmp, len);
	float val = 0.0;
	if (len != 0) {
		val = (float) atof(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field_assoc_int: Can't find field %s or result is empty.", fieldname);
	}
	free(fieldname);
	amxMutex->unlock();
	return amx_ftoc(val);
}