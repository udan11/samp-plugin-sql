/**
 * SA:MP Plugin - MySQL
 * Copyright (C) 2013 Dan
 *  
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "natives.h"

cell AMX_NATIVE_CALL Natives::mysql_debug(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	log_level_file = params[1];
	log_level_console = params[2];
	log(LOG_WARNING, "Natives::mysql_debug: Switching the log levels to (%d, %d)...", log_level_file, log_level_console);
	Mutex::getInstance()->unlock();
	return 1;
}

cell AMX_NATIVE_CALL Natives::mysql_connect(AMX *amx, cell *params) {
	if (params[0] < 5 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	MySQL_Handler *handler;
	handler = new MySQL_Handler();
	handler->amx = amx;
	int id = last_handler++;
	char *host = NULL, *user = NULL, *pass = NULL, *db = NULL;
	amx_GetString_(amx, params[1], host);
	amx_GetString_(amx, params[2], user);
	amx_GetString_(amx, params[3], pass);
	amx_GetString_(amx, params[4], db);
	int port = (int) params[5];
	log(LOG_INFO, "Natives::mysql_connect: Connecting to %s:***@%s:%d/%s...", user, host, port, db);
	bool connected = handler->connect(host, user, pass, db, port);
	free(host);
	free(user);
	free(pass);
	free(db);
	if (connected) {
		log(LOG_INFO, "Natives::mysql_connect: Connection was succesful!");
		handlers[id] = handler;
		Mutex::getInstance()->unlock();
		return id;
	}
	log(LOG_INFO, "Natives::mysql_connect: Connection failed! (error = %d, %s)", handler->get_errno(), handler->get_error());
	delete handler;
	Mutex::getInstance()->unlock();
	return 0;
}

cell AMX_NATIVE_CALL Natives::mysql_disconnect(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	delete handlers[handler_id];
	handlers.erase(handler_id);
	log(LOG_INFO, "Natives::mysql_disconnect: Handler %d was destroyed!", handler_id);
	Mutex::getInstance()->unlock();
	return 1;
}

cell AMX_NATIVE_CALL Natives::mysql_set_charset(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	char *charset = NULL;
	amx_GetString_(amx, params[2], charset);
	bool ret = handlers[handler_id]->set_charset(charset);
	free(charset);
	Mutex::getInstance()->unlock();
	return ret;
}

cell AMX_NATIVE_CALL Natives::mysql_get_charset(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	char *tmp = (char*) handlers[handler_id]->get_charset();
	int dest_len = params[3], len = strlen(tmp);
	if (dest_len < 2) {
		amx_SetString_(amx, params[2], tmp, len);
	} else {
		amx_SetString_(amx, params[2], tmp, dest_len);
	}
	return 1;
}

cell AMX_NATIVE_CALL Natives::mysql_ping(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return -1;
	}
	Mutex::getInstance()->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		Mutex::getInstance()->unlock();
		return -1;
	}
	log(LOG_DEBUG, "Natives::mysql_ping: Pinging handled %d...", handler_id);
	int ping = handlers[handler_id]->ping();
	Mutex::getInstance()->unlock();
	return ping;
}

cell AMX_NATIVE_CALL Natives::mysql_get_stat(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	char *tmp = (char*) handlers[handler_id]->get_stat();
	int dest_len = params[3], len = strlen(tmp);
	if (dest_len < 2) {
		amx_SetString_(amx, params[2], tmp, len);
	} else {
		amx_SetString_(amx, params[2], tmp, dest_len);
	}
	return 1;
}

cell AMX_NATIVE_CALL Natives::mysql_escape_string(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return -1;
	}
	Mutex::getInstance()->lock();
	int handler_id = params[1];
	if (!is_valid_handler(handler_id)) {
		Mutex::getInstance()->unlock();
		return -1;
	}
	char *src = NULL;
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
	Mutex::getInstance()->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::mysql_query(AMX *amx, cell *params) {
	if (params[0] < 5 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	struct mysql_query *query = (struct mysql_query*) malloc(sizeof(struct mysql_query));
	if (query == NULL) {
		log(LOG_WARNING, "Cannot allocate memory.");
		return 0;
	}
	memset(query, 0, sizeof(struct mysql_query));
	int id = last_query++;
	query->id = id;
	query->handler = params[1];
	if (!is_valid_handler(query->handler)) {
		free_query(query);
		Mutex::getInstance()->unlock();
		return 0;
	}
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
	log(LOG_DEBUG, "Natives::mysql_query: Scheduling query %d: \"%s\", callback: %s(%s) for execution...", query->id, query->query, query->callback, query->format);
	queries[query->id] = query;
	if (!(query->flags & QUERY_THREADED)) {
		log(LOG_DEBUG, "Natives::mysql_query: Executing query->id = %d...", query->id);
		handlers[query->handler]->execute_query(query);
		if (strlen(query->callback)) {
			log(LOG_DEBUG, "Natives::mysql_query: Executing query callback (query->error = %d)...", query->error);
			execute_query_callback(query);
		}
		Mutex::getInstance()->unlock();
		return id;
	}
	Mutex::getInstance()->unlock();
	return id;
}

cell AMX_NATIVE_CALL Natives::mysql_free_result(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	struct mysql_query *query = queries[query_id];
	log(LOG_DEBUG, "Natives::mysql_query: Freeing query %d...", query->id);
	free_query(query);
	queries.erase(query_id);
	Mutex::getInstance()->unlock();
	return 1;
}

cell AMX_NATIVE_CALL Natives::mysql_store_result(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	struct mysql_query *query = queries[query_id];
	log(LOG_DEBUG, "Natives::mysql_query: Storing query %d...", query->id);
	query->flags &= ~QUERY_THREADED;
	query->status = QUERY_STATUS_EXECUTED;
	Mutex::getInstance()->unlock();
	return 1;
}

cell AMX_NATIVE_CALL Natives::mysql_insert_id(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	int insert_id = queries[query_id]->insert_id;
	Mutex::getInstance()->unlock();
	return insert_id;
}

cell AMX_NATIVE_CALL Natives::mysql_affected_rows(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	int affected_rows = queries[query_id]->affected_rows;
	Mutex::getInstance()->unlock();
	return affected_rows;
}

cell AMX_NATIVE_CALL Natives::mysql_error(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	int error = queries[query_id]->error;
	Mutex::getInstance()->unlock();
	return error;
}

cell AMX_NATIVE_CALL Natives::mysql_error_string(AMX *amx, cell *params) {
	if (params[0] < 3 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	struct mysql_query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		Mutex::getInstance()->unlock();
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
	Mutex::getInstance()->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::mysql_num_rows(AMX *amx, cell *params) {
	if (params[0] < 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	int num_rows = queries[query_id]->num_rows;
	Mutex::getInstance()->unlock();
	return num_rows;
}

cell AMX_NATIVE_CALL Natives::mysql_num_fields(AMX *amx, cell *params) {
	if (params[0] < 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	int num_fields = queries[query_id]->num_fields;
	Mutex::getInstance()->unlock();
	return num_fields;
}

cell AMX_NATIVE_CALL Natives::mysql_field_name(AMX *amx, cell *params) {
	if (params[0] < 4 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	struct mysql_query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	int dest_len = params[5], len;
	char *tmp = NULL;
	bool isCopy = handlers[query->handler]->fetch_field(query, params[2], tmp, len);
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
		log(LOG_WARNING, "Natives::mysql_field_name: Can't find field %d or result is empty.", params[2]);
	}
	Mutex::getInstance()->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::mysql_next_row(AMX *amx, cell* params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	struct mysql_query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	int ret = handlers[query->handler]->seek_row(query, params[2]);
	Mutex::getInstance()->unlock();
	return ret;
}

cell AMX_NATIVE_CALL Natives::mysql_get_field(AMX *amx, cell *params) {
	if (params[0] < 4 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	struct mysql_query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	int fieldidx = params[2], dest_len = params[4], len;
	char *tmp = NULL;
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
		log(LOG_WARNING, "Natives::mysql_get_field: Can't find field %d or result is empty.", fieldidx);
	}
	Mutex::getInstance()->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::mysql_get_field_assoc(AMX *amx, cell *params) {
	if (params[0] < 4 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	struct mysql_query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	char *fieldname = NULL, *tmp = NULL;
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
		log(LOG_WARNING, "Natives::mysql_get_field_assoc: Can't find field %s or result is empty.", fieldname);
	}
	free(fieldname);
	Mutex::getInstance()->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::mysql_get_field_int(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	struct mysql_query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	int fieldidx = params[2], len, val = 0;
	char *tmp = NULL;
	bool isCopy = handlers[query->handler]->fetch_num(query, fieldidx, tmp, len);
	if (len != 0) {
		val = atoi(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::mysql_get_field_int: Can't find field %d or result is empty.", params[2]);
	}
	Mutex::getInstance()->unlock();
	return val;
}

cell AMX_NATIVE_CALL Natives::mysql_get_field_assoc_int(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	struct mysql_query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	char *fieldname = NULL, *tmp = NULL;
	amx_GetString_(amx, params[2], fieldname);
	int len, val = 0;
	bool isCopy = handlers[query->handler]->fetch_assoc(query, fieldname, tmp, len);
	if (len != 0) {
		val = atoi(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::mysql_get_field_assoc_int: Can't find field %s or result is empty.", fieldname);
	}
	free(fieldname);
	Mutex::getInstance()->unlock();
	return val;
}

cell AMX_NATIVE_CALL Natives::mysql_get_field_float(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	struct mysql_query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	int fieldidx = params[2], len;
	char *tmp = NULL;
	bool isCopy = handlers[query->handler]->fetch_num(query, fieldidx, tmp, len);
	float val = 0.0;
	if (len != 0) {
		val = (float) atof(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		log(LOG_WARNING, "Natives::mysql_get_field_int: Can't find field %d or result is empty.", params[2]);
	}
	Mutex::getInstance()->unlock();
	return amx_ftoc(val);
}

cell AMX_NATIVE_CALL Natives::mysql_get_field_assoc_float(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	Mutex::getInstance()->lock();
	int query_id = params[1];
	if (!is_valid_query(query_id)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	struct mysql_query *query = queries[query_id];
	if (!is_valid_handler(query->handler)) {
		Mutex::getInstance()->unlock();
		return 0;
	}
	char *fieldname = NULL, *tmp = NULL;
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
		log(LOG_WARNING, "Natives::mysql_get_field_assoc_int: Can't find field %s or result is empty.", fieldname);
	}
	free(fieldname);
	Mutex::getInstance()->unlock();
	return amx_ftoc(val);
}