#include "natives.h"

cell AMX_NATIVE_CALL Natives::mysql_debug(AMX *amx, cell *params) {
	if (params[0] / 4 < 1) {
		return 0;
	}
	Mutex::getInstance()->lock();
	log_level = params[1];
	log(LOG_WARNING, "Natives::mysql_debug: Switching the log level to %d...", log_level);
	Mutex::getInstance()->unlock();
	return 1;
}

cell AMX_NATIVE_CALL Natives::mysql_connect(AMX *amx, cell *params) {
	if (params[0] / 4 < 5) {
		return 0;
	}
	Mutex::getInstance()->lock();
	MySQL_Handler *handler;
	handler = new MySQL_Handler();
	handler->amx = amx;
	int id = last_handler++;
	handler->id = id;
	char *host = NULL, *user = NULL, *pass = NULL, *db = NULL;
	amx_GetString_(amx, params[1], host);
	amx_GetString_(amx, params[2], user);
	amx_GetString_(amx, params[3], pass);
	amx_GetString_(amx, params[4], db);
	int port = (int) params[5];
	log(LOG_INFO, "Natives::mysql_connect: Connecting to %s:***@%s:%d/%s...", user, host, port, db);
	if (handler->connect(host, user, pass, db, port)) {
		log(LOG_INFO, "Natives::mysql_connect: Connection was succesful!");
		handlers[handler->id] = handler;
		Mutex::getInstance()->unlock();
		return id;
	}
	log(LOG_INFO, "Natives::mysql_connect: Connection failed! (error = %d, %s)", handler->get_errno(), handler->get_error());
	delete handler;
	Mutex::getInstance()->unlock();
	return 0;
}

cell AMX_NATIVE_CALL Natives::mysql_disconnect(AMX *amx, cell *params) {
	if (params[0] / 4 < 1) {
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

cell AMX_NATIVE_CALL Natives::mysql_ping(AMX *amx, cell *params) {
	if (params[0] / 4 < 1) {
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

cell AMX_NATIVE_CALL Natives::mysql_escape_string(AMX *amx, cell *params) {
	if (params[0] / 4 < 1) {
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
	char *dest = (char*) malloc(2 * sizeof(char) *  strlen(src));
	int dest_len = params[4], len = handlers[handler_id]->escape_string(src, dest);
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
	if (params[0] / 4 < 5) {
		return 0;
	}
	Mutex::getInstance()->lock();
	struct mysql_query *query;
	query = (struct mysql_query*) malloc(sizeof(struct mysql_query));
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
		cell *ptr;
		switch (query->format[i]) {
			case 'c':
			case 'C':
			case 'd':
			case 'D':
			case 'i':
			case 'I':
				amx_GetAddr(amx, params[p], &ptr);
				query->params_i.push_back((int) *ptr);
				break;
			case 'f': 
			case 'F':
				amx_GetAddr(amx, params[p], &ptr);
				query->params_f.push_back(amx_ctof(*ptr));
				break;
			case 'r':
			case 'R':
				// We didn't read any param.
				--p;
				break;
			case 's':
			case 'S':
				char *str;
				amx_GetString_(amx, params[p], str);
				query->params_s.push_back(str);
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
	return 0;
}

cell AMX_NATIVE_CALL Natives::mysql_free_result(AMX *amx, cell *params) {
	if (params[0] / 4 < 1) {
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
	if (params[0] / 4 < 1) {
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
	if (params[0] / 4 < 1) {
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
	if (params[0] / 4 < 1) {
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
	if (params[0] / 4 < 1) {
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
	// TODO
	return 0;
}

cell AMX_NATIVE_CALL Natives::mysql_num_rows(AMX *amx, cell *params) {
	if (params[0] / 4 < 1) {
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
	if (params[0] / 4 < 1) {
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
	if (params[0] / 4 < 4) {
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
	char *dest = NULL;
	int dest_len = params[5], len = handlers[query->handler]->fetch_field(query, params[2], dest);
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

cell AMX_NATIVE_CALL Natives::mysql_next_row(AMX *amx, cell* params) {
	if (params[0] / 4 < 2) {
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
	if (params[0] / 4 < 4) {
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
	char *dest = NULL;
	int dest_len = params[4], len = handlers[query->handler]->fetch_num(query, params[2], dest);
	if (len != 0) {
		if (dest_len < 2) {
			amx_SetString_(amx, params[3], dest, len);
		} else {
			amx_SetString_(amx, params[3], dest, dest_len);
		}
		free(dest);
	} else {
		log(LOG_WARNING, "Natives::mysql_get_field: Can't find field %d.", query_id, params[2]);
	}
	Mutex::getInstance()->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::mysql_get_field_assoc(AMX *amx, cell *params) {
	if (params[0] / 4 < 4) {
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
	char *fieldname = NULL, *dest = NULL;
	amx_GetString_(amx, params[2], fieldname);
	int dest_len = params[4], len = handlers[query->handler]->fetch_assoc(query, fieldname, dest);
	if (len != 0) {
		if (dest_len < 2) {
			amx_SetString_(amx, params[3], dest, len);
		} else {
			amx_SetString_(amx, params[3], dest, dest_len);
		}
		free(dest);
	} else {
		log(LOG_WARNING, "Natives::mysql_get_field_assoc: Can't find field %s.", query_id, fieldname);
	}
	Mutex::getInstance()->unlock();
	return len;
}

cell AMX_NATIVE_CALL Natives::mysql_get_field_int(AMX *amx, cell *params) {
	if (params[0] / 4 < 2) {
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
	char *dest = NULL;
	int len = handlers[query->handler]->fetch_num(query, params[2], dest);
	if (len != 0) {
		int val = atoi(dest);
		free(dest);
		Mutex::getInstance()->unlock();
		return val;
	} else {
		log(LOG_WARNING, "Natives::mysql_get_field_int: Can't find field %d.", query_id, params[2]);
	}
	Mutex::getInstance()->unlock();
	return 0;
}

cell AMX_NATIVE_CALL Natives::mysql_get_field_assoc_int(AMX *amx, cell *params) {
	if (params[0] / 4 < 2) {
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
	char *fieldname = NULL, *dest = NULL;
	amx_GetString_(amx, params[2], fieldname);
	int len = handlers[query->handler]->fetch_assoc(query, fieldname, dest);
	if (len != 0) {
		int val = atoi(dest);
		free(dest);
		Mutex::getInstance()->unlock();
		return val;
	} else {
		log(LOG_WARNING, "Natives::mysql_get_field_assoc_int: Can't find field %s.", query_id, fieldname);
	}
	Mutex::getInstance()->unlock();
	return 0;
}

cell AMX_NATIVE_CALL Natives::mysql_get_field_float(AMX *amx, cell *params) {
	if (params[0] / 4 < 2) {
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
	char *dest = NULL;
	int len = handlers[query->handler]->fetch_num(query, params[2], dest);
	if (len != 0) {
		float val = (float) atof(dest);
		free(dest);
		Mutex::getInstance()->unlock();
		return amx_ftoc(val);
	} else {
		log(LOG_WARNING, "Natives::mysql_get_field_float: Can't find field %d.", query_id, params[2]);
	}
	Mutex::getInstance()->unlock();
	return 0;
}

cell AMX_NATIVE_CALL Natives::mysql_get_field_assoc_float(AMX *amx, cell *params) {
	if (params[0] / 4 < 2) {
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
	char *fieldname = NULL, *dest = NULL;
	amx_GetString_(amx, params[2], fieldname);
	int len = handlers[query->handler]->fetch_assoc(query, fieldname, dest);
	if (len != 0) {
		float val = (float) atof(dest);
		free(dest);
		Mutex::getInstance()->unlock();
		return amx_ftoc(val);
	} else {
		log(LOG_WARNING, "Natives::mysql_get_field_assoc_float: Can't find field %s.", query_id, fieldname);
	}
	Mutex::getInstance()->unlock();
	return 0;
}