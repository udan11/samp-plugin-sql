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

#include <cstdio>

#include "sdk/amx/amx.h"
#include "sdk/amxstring.h"

#include "sql/sql.h"
#include "mysql/mysql.h"
#include "pgsql/pgsql.h"

#include "log.h"

#include "natives.h"

cell AMX_NATIVE_CALL Natives::sql_debug(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	logFile = params[1];
	logConsole = params[2];
	log(LOG_WARNING, "Natives::sql_debug: Switching the log levels to (%d, %d)...", logFile, logConsole);
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_connect(AMX *amx, cell *params) {
	if (params[0] < 6 * 4) {
		return 0;
	}
	int id = lastHandler++;
	SQL_Handler *handler = NULL;
	switch (params[1]) {
#ifdef SQL_HANDLER_MYSQL
		case SQL_HANDLER_MYSQL:
			handler = new MySQL_Handler(id, amx);
			break;
#endif
#ifdef SQL_HANDLER_PGSQL
		case SQL_HANDLER_PGSQL:
			handler = new PgSQL_Handler(id, amx);
			break;
#endif
		default:
			log(LOG_ERROR, "Natives::sql_connect: Unknown SQL type (%d)!", params[1]);
			return 0;
	}
	char *host = NULL, *user = NULL, *pass = NULL, *db = NULL;
	amx_StrParam(amx, params[2], host);
	amx_StrParam(amx, params[3], user);
	amx_StrParam(amx, params[4], pass);
	amx_StrParam(amx, params[5], db);
	if (host == NULL) {
		log(LOG_WARNING, "Natives::sql_connect: The `host` field is empty.");
	}
	if (user == NULL) {
		log(LOG_WARNING, "Natives::sql_connect: The `user` field is empty.");
	}
	if (pass == NULL) {
		log(LOG_WARNING, "Natives::sql_connect: The `pass` field is empty.");
	}
	if (db == NULL) {
		log(LOG_WARNING, "Natives::sql_connect: The `db` field is empty.");
	}
	log(LOG_INFO, "Natives::sql_connect: Connecting to database (type = %d) %s:***@%s:%d/%s...", params[1], user, host, params[6], db);
	if (handler->connect(host, user, pass, db, params[6])) {
		log(LOG_INFO, "Natives::sql_connect: Connection (handler->id = %d) was succesful!", id);
		handlers[id] = handler;
	} else {
		log(LOG_WARNING, "Natives::sql_connect: Connection (handler->id = %d) failed! (error = %d, %s)", id, handler->get_errno(), handler->get_error());
		delete handler;
		id = 0;
	}
	return id;
}

cell AMX_NATIVE_CALL Natives::sql_disconnect(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!is_valid_handler(params[1])) {
		return 0;
	}
	SQL_Handler *handler = handlers[params[1]];
	handlers.erase(params[1]);
	delete handler;
	log(LOG_INFO, "Natives::sql_disconnect: Handler (handler->id = %d) was destroyed!", params[1]);
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_wait(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!is_valid_handler(params[1])) {
		return 0;
	}
	SQL_Handler *handler = handlers[params[1]];
	while (!handler->pending.empty()) {
		SLEEP(TICK_RATE);
	}
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_set_charset(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	if (!is_valid_handler(params[1])) {
		return 0;
	}
	char *charset = NULL;
	amx_StrParam(amx, params[2], charset);
	if (charset == NULL) {
		log(LOG_WARNING, "Natives::sql_set_charset: New charset is null.");
		return false;
	}
	log(LOG_INFO, "Natives::sql_set_charset: Setting handler's charset (handler->id = %d) to %s.", params[1], charset);
	return handlers[params[1]]->set_charset(charset);
}

cell AMX_NATIVE_CALL Natives::sql_get_charset(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	if (!is_valid_handler(params[1])) {
		return 0;
	}
	char *tmp = (char*) handlers[params[1]]->get_charset();
	log(LOG_DEBUG, "Natives::sql_get_charset: Getting handler's charset (handler->id = %d, handler->get_charset() = %s)...", params[1], tmp);
	int len = strlen(tmp);
	if (params[3] < 2) {
		log(LOG_DEBUG, "Natives::sql_get_charset: Specified destination size is smaller than 2, setting it to %s.", len);
		amx_SetString_(amx, params[2], tmp, len);
	} else {
		amx_SetString_(amx, params[2], tmp, params[3]);
	}
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_ping(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return -1;
	}
	if (!is_valid_handler(params[1])) {
		return -1;
	}
	log(LOG_DEBUG, "Natives::sql_ping: Pinging handler (handler->id = %d)...", params[1]);
	return handlers[params[1]]->ping();
}

cell AMX_NATIVE_CALL Natives::sql_get_stat(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	if (!is_valid_handler(params[1])) {
		return 0;
	}
	char *tmp = (char*) handlers[params[1]]->get_stat();
	log(LOG_DEBUG, "Natives::sql_get_stat: Getting handler's statistics (handler->id = %d, handler->get_stat() = %s)...", params[1], tmp);
	int len = strlen(tmp);
	if (params[3] < 2) {
		log(LOG_DEBUG, "Natives::sql_get_stat: Specified destination size is smaller than 2, setting it to %s.", len);
		amx_SetString_(amx, params[2], tmp, len);
	} else {
		amx_SetString_(amx, params[2], tmp, params[3]);
	}
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_escape_string(AMX *amx, cell *params) {
	if (params[0] < 3 * 4) {
		return -1;
	}
	char *src = NULL;
	amx_StrParam(amx, params[2], src);
	if (src == NULL) {
		log(LOG_DEBUG, "Natives::sql_escape_string: Escaping null string...");
		amx_SetString_(amx, params[3], "", params[4]);
		return 0;
	}
	if (!is_valid_handler(params[1])) {
		return -1;
	}
	log(LOG_DEBUG, "Natives::sql_escape_string: Escaping (handler->id = %d) string '%s'...", params[1], src);
	char *dest = (char*) malloc(sizeof(char) * strlen(src) * 2); // *2 in case every character is escaped
	if (dest != NULL) {
		int len = handlers[params[1]]->escape_string(src, dest);
		if (len != 0) {
			if (params[4] < 2) {
				log(LOG_DEBUG, "Natives::sql_escape_string: Specified destination size is smaller than 2, setting it to %s.", len);
				amx_SetString_(amx, params[3], dest, len);
			} else {
				amx_SetString_(amx, params[3], dest, params[4]);
			}
			free(dest);
		}
		return len;
	}
	return 0;
}

cell AMX_NATIVE_CALL Natives::sql_query(AMX *amx, cell *params) {
	if (params[0] < 5 * 4) {
		return 0;
	}
	if (!is_valid_handler(params[1])) {
		log(LOG_WARNING, "Natives::sql_query: Invalid handler! (handler->id = %d)", params[1]);
		return 0;
	}
	SQL_Query *query;
	switch (handlers[params[1]]->type) {
#ifdef SQL_HANDLER_MYSQL
		case SQL_HANDLER_MYSQL:
			query = new MySQL_Query();
			break;
#endif
#ifdef SQL_HANDLER_PGSQL
		case SQL_HANDLER_PGSQL:
			query = new PgSQL_Query();
			break;
#endif
		default:
			log(LOG_WARNING, "Natives::sql_query: Invalid handler! (handler->id = %d, handler->type = %d)", params[1], handlers[params[1]]->type);
			return 0;
	}
	int id = lastQuery++;
	query->amx = amx;
	query->id = id;
	query->handler = params[1];
	amx_GetString_(amx, params[2], query->query);
	query->flags = params[3];
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
				if (arr != NULL) {
					memcpy(arr, ptr_arr, len);
					query->paramsArr.push_back(std::make_pair(arr, *ptr_len));
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
				query->paramsC.push_back(*ptr);
				break;
			case 'r':
			case 'R':
				--p; // We didn't read any parameter.
				break;
			case 's':
			case 'S':
				char *str;
				amx_GetString_(amx, params[p], str);
				query->paramsStr.push_back(str);
				break;
			default: 
				log(LOG_WARNING, "Natives::sql_query: Format '%c' is not recognized.", query->format[i]);
				break;
		}
	}
	queries[query->id] = query;
	SQL_Handler *handler = handlers[query->handler];
	if (query->flags & QUERY_THREADED) {
		log(LOG_DEBUG, "Natives::sql_query: Scheduling query (query->id = %d, query->query = %s, query->callback = %s) for execution...", query->id, query->query, query->callback);
		handler->pending.push(query);
	} else {
		log(LOG_DEBUG, "Natives::sql_query: Executing query (query->id = %d, query->query = %s)...", query->id, query->query);
		handler->execute_query(query);
		if ((strlen(query->callback)) || (query->error != 0)) {
			log(LOG_DEBUG, "Natives::sql_query: Executing query callback (query->id = %d, query->error = %d, query->callback = %s)...", query->id, query->error, query->callback);
			query->execute_callback();
			if (!is_valid_query(id)) {
				id = 0;
			}
		} else {
			log(LOG_DEBUG, "Natives::sql_query: Query executed (query->id = %d, query->error = %d). No callback found!", query->id, query->error);
		}
	}
	return id;
}

cell AMX_NATIVE_CALL Natives::sql_free_result(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	log(LOG_DEBUG, "Natives::sql_free_result: Freeing query (query->id = %d)...", params[1]);
	queries.erase(params[1]);
	delete query;
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_store_result(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	log(LOG_DEBUG, "Natives::sql_store_result: Storing query (query->id = %d)...", params[1]);
	// Switching the state of this query to non-threaded (it has to be freed manually).
	query->flags &= ~QUERY_THREADED;
	query->status = QUERY_STATUS_EXECUTED;
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_insert_id(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	log(LOG_DEBUG, "Natives::sql_insert_id: Retrieving insert ID (query->id = %d)...", params[1]);
	return query->results[query->lastResultIdx]->insertId;
}

cell AMX_NATIVE_CALL Natives::sql_affected_rows(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	log(LOG_DEBUG, "Natives::sql_affected_rows: Retrieving the count of affected rows (query->id = %d)...", params[1]);
	return query->results[query->lastResultIdx]->affectedRows;
}

cell AMX_NATIVE_CALL Natives::sql_error(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	log(LOG_DEBUG, "Natives::sql_error: Retrieving error code (query->id = %d)...", params[1]);
	return query->error;
}

cell AMX_NATIVE_CALL Natives::sql_error_string(AMX *amx, cell *params) {
	if (params[0] < 3 * 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	if (!is_valid_handler(query->handler)) {
		return 0;
	}
	int len = strlen(query->errorMsg);
	if (len != 0) {
		char *error = (char*) malloc(sizeof(char) * len);
		strcpy(error, query->errorMsg);
		if (params[3] < 2) {
			log(LOG_DEBUG, "Natives::sql_error_string: Specified destination size is smaller than 2, setting it to %s.", len);
			amx_SetString_(amx, params[2], error, len);
		} else {
			amx_SetString_(amx, params[2], error, params[3]);
		}
	}
	log(LOG_DEBUG, "Natives::sql_error_string: Retrieving error string (query->id = %d)...", params[1]);
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_num_rows(AMX *amx, cell *params) {
	if (params[0] < 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	log(LOG_DEBUG, "Natives::sql_num_rows: Retrieving the count of rows (query->id = %d)...", params[1]);
	return query->results[query->lastResultIdx]->numRows;
}

cell AMX_NATIVE_CALL Natives::sql_num_fields(AMX *amx, cell *params) {
	if (params[0] < 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	log(LOG_DEBUG, "Natives::sql_num_fields: Retrieving the count of fields (query->id = %d)...", params[1]);
	return query->results[query->lastResultIdx]->numFields;
}

cell AMX_NATIVE_CALL Natives::sql_next_result(AMX *amx, cell* params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	if (!is_valid_handler(query->handler)) {
		return 0;
	}
	log(LOG_DEBUG, "Natives::sql_next_result: Retrieving next result (query->id = %d, next_result = %d)...", params[1], params[2]);
	return handlers[query->handler]->seek_result(query, params[2]);
}

cell AMX_NATIVE_CALL Natives::sql_field_name(AMX *amx, cell *params) {
	if (params[0] < 4 * 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	if (!is_valid_handler(query->handler)) {
		return 0;
	}
	char *tmp = NULL;
	int len;
	bool isCopy = handlers[query->handler]->fetch_field(query, params[2], tmp, len);
	if (len != 0) {
		if (params[4] < 2) {
			log(LOG_DEBUG, "Natives::sql_field_name: Specified destination size is smaller than 2, setting it to %s.", len);
			amx_SetString_(amx, params[3], tmp, len);
		} else {
			amx_SetString_(amx, params[3], tmp, params[4]);
		}
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_field_name: Can't find field %d or result is empty.", params[2]);
	}
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_fetch_row(AMX *amx, cell *params) {
	if (params[0] < 4 * 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	if (!is_valid_handler(query->handler)) {
		return 0;
	}
	SQL_Handler *handler = handlers[query->handler];
	log(LOG_DEBUG, "Natives::sql_fetch_row: Fetching a row (query->id = %d)...", params[1]);
	char *sep;
	amx_StrParam(amx, params[2], sep);
	char *ret = (char*) malloc(4096 * sizeof(char));
	memset(ret, 0, 4096 * sizeof(char));
	for (int i = 0, fields = query->results[query->lastResultIdx]->numFields; i != fields; ++i) {
		char *tmp = NULL;
		int len;
		bool isCopy = handler->fetch_num(query, i, tmp, len);
		sprintf(ret, "%s%s%s", ret, tmp, sep);
		if (isCopy) {
			free(tmp);
		}
	}
	int len = strlen(ret);
	if (len != 0) {
		if (params[4] < 2) { // Probably a multi-dimensional array.
			log(LOG_DEBUG, "Natives::sql_fetch_row: Specified destination size is smaller than 2, setting it to %s.", len);
			amx_SetString_(amx, params[3], ret, len);
		} else {
			amx_SetString_(amx, params[3], ret, params[4]);
		}
		free(ret);
	} else {
		log(LOG_WARNING, "Natives::sql_fetch_row: This row is empty.");
	}
	return 0;
}

cell AMX_NATIVE_CALL Natives::sql_next_row(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	if (!is_valid_handler(query->handler)) {
		return 0;
	}
	log(LOG_DEBUG, "Natives::sql_next_row: Retrieving next row (query->id = %d, next_row = %d)...", params[1], params[2]);
	return handlers[query->handler]->seek_row(query, params[2]);
}

cell AMX_NATIVE_CALL Natives::sql_get_field(AMX *amx, cell *params) {
	cell fieldidx, row, dest_str, dest_len;
	if (params[0] == 4 * 4) {
		row = -1;
		fieldidx = params[2];
		dest_str = params[3];
		dest_len = params[4];
	} else if (params[0] == 5 * 4) {
		row = params[2];
		fieldidx = params[3];
		dest_str = params[4];
		dest_len = params[5];
	} else {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	if (!is_valid_handler(query->handler)) {
		return 0;
	}
	SQL_Handler *handler = handlers[query->handler];
	if (row != -1) {
		handler->seek_row(query, row);
	}
	char *tmp = NULL;
	int len;
	bool isCopy = handler->fetch_num(query, fieldidx, tmp, len);
	if (len != 0) {
		if (dest_len < 2) { // Probably a multi-dimensional array.
			log(LOG_DEBUG, "Natives::sql_get_field: Specified destination size is smaller than 2, setting it to %s.", len);
			amx_SetString_(amx, dest_str, tmp, len);
		} else {
			amx_SetString_(amx, dest_str, tmp, dest_len);
		}
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field: Can't find field %d or result is empty.", fieldidx);
	}
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_assoc(AMX *amx, cell *params) {
	cell fieldidx, row, dest_str, dest_len;
	if (params[0] == 4 * 4) {
		row = -1;
		fieldidx = params[2];
		dest_str = params[3];
		dest_len = params[4];
	} else if (params[0] == 5 * 4) {
		row = params[2];
		fieldidx = params[3];
		dest_str = params[4];
		dest_len = params[5];
	} else {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	if (!is_valid_handler(query->handler)) {
		return 0;
	}
	SQL_Handler *handler = handlers[query->handler];
	if (row != -1) {
		handler->seek_row(query, row);
	}
	char *fieldname = NULL;
	amx_StrParam(amx, fieldidx, fieldname);
	if (fieldname == NULL) {
		log(LOG_WARNING, "Natives::sql_get_field_assoc: Field name is empty.");
		amx_SetString_(amx, dest_str, "", dest_len);
		return 0;
	}
	char *tmp = NULL;
	int len;
	bool isCopy = handler->fetch_assoc(query, fieldname, tmp, len);
	if (len != 0) {
		if (dest_len < 2) {
			log(LOG_DEBUG, "Natives::sql_get_field_assoc: Specified destination size is smaller than 2, setting it to %s.", len);
			amx_SetString_(amx, dest_str, tmp, len);
		} else {
			amx_SetString_(amx, dest_str, tmp, dest_len);
		}
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field_assoc: Can't find field %s or result is empty.", fieldname);
	}
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_int(AMX *amx, cell *params) {
	cell fieldidx, row;
	if (params[0] == 2 * 4) {
		row = -1;
		fieldidx = params[2];
	} else if (params[0] == 3 * 4) {
		row = params[2];
		fieldidx = params[3];
	} else {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	if (!is_valid_handler(query->handler)) {
		return 0;
	}
	SQL_Handler *handler = handlers[query->handler];
	if (row != -1) {
		handler->seek_row(query, row);
	}
	char *tmp = NULL;
	int len, val = 0;
	bool isCopy = handler->fetch_num(query, fieldidx, tmp, len);
	if (len != 0) {
		val = atoi(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field_int: Can't find field %d or result is empty.", fieldidx);
	}
	return val;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_assoc_int(AMX *amx, cell *params) {
	cell fieldidx, row;
	if (params[0] == 2 * 4) {
		row = -1;
		fieldidx = params[2];
	} else if (params[0] == 3 * 4) {
		row = params[2];
		fieldidx = params[3];
	} else {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	if (!is_valid_handler(query->handler)) {
		return 0;
	}
	SQL_Handler *handler = handlers[query->handler];
	if (row != -1) {
		handler->seek_row(query, row);
	}
	char *fieldname = NULL;
	amx_StrParam(amx, fieldidx, fieldname);
	if (fieldname == NULL) {
		log(LOG_WARNING, "Natives::sql_get_field_assoc: Field name is empty.");
		return 0;
	}
	char *tmp = NULL;
	int len, val = 0;
	bool isCopy = handler->fetch_assoc(query, fieldname, tmp, len);
	if (len != 0) {
		val = atoi(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field_assoc_int: Can't find field %s or result is empty.", fieldname);
	}
	return val;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_float(AMX *amx, cell *params) {
	cell query_id, fieldidx, row;
	if (params[0] == 2 * 4) {
		row = -1;
		fieldidx = params[2];
	} else if (params[0] == 3 * 4) {
		row = params[2];
		fieldidx = params[3];
	} else {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	if (!is_valid_handler(query->handler)) {
		return 0;
	}
	SQL_Handler *handler = handlers[query->handler];
	if (row != -1) {
		handler->seek_row(query, row);
	}
	int len;
	char *tmp = NULL;
	bool isCopy = handler->fetch_num(query, fieldidx, tmp, len);
	float val = 0.0;
	if (len != 0) {
		val = (float) atof(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field_int: Can't find field %d or result is empty.", fieldidx);
	}
	return amx_ftoc(val);
}

cell AMX_NATIVE_CALL Natives::sql_get_field_assoc_float(AMX *amx, cell *params) {
	cell fieldidx, row;
	if (params[0] == 2 * 4) {
		row = -1;
		fieldidx = params[2];
	} else if (params[0] == 3 * 4) {
		row = params[2];
		fieldidx = params[3];
	} else {
		return 0;
	}
	if (!is_valid_query(params[1])) {
		return 0;
	}
	SQL_Query *query = queries[params[1]];
	if (query->status == QUERY_STATUS_NONE) {
		return 0;
	}
	if (!is_valid_handler(query->handler)) {
		return 0;
	}
	SQL_Handler *handler = handlers[query->handler];
	if (row != -1) {
		handler->seek_row(query, row);
	}
	char *fieldname = NULL;
	amx_StrParam(amx, fieldidx, fieldname);
	if (fieldname == NULL) {
		log(LOG_WARNING, "Natives::sql_get_field_assoc: Field name is empty.");
		return 0;
	}
	char *tmp = NULL;
	int len;
	bool isCopy = handler->fetch_assoc(query, fieldname, tmp, len);
	float val = 0.0;
	if (len != 0) {
		val = (float) atof(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::sql_get_field_assoc_int: Can't find field %s or result is empty.", fieldname);
	}
	return amx_ftoc(val);
}
