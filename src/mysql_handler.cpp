#include "mysql_handler.h"

AMX *amx;
int id;
MYSQL *conn;

MySQL_Handler::MySQL_Handler() {
	amx = NULL;
	id = 0;
	conn = mysql_init(NULL);
	int arg = 1;
	mysql_options(conn, MYSQL_OPT_RECONNECT, &arg);
}

MySQL_Handler::~MySQL_Handler() {
	disconnect();
}

bool MySQL_Handler::connect(const char *host, const char *user, const char *pass, const char *db, int port = 3306) {
	return mysql_real_connect(conn, host, user, pass, db, port, NULL, 0) ? true : false;
}

void MySQL_Handler::disconnect() {
	mysql_close(conn);
}

int MySQL_Handler::get_errno() {
	return mysql_errno(conn);
}

const char* MySQL_Handler::get_error() {
	return mysql_error(conn);
}

int MySQL_Handler::ping() {
	return mysql_ping(conn);
}

const char* MySQL_Handler::get_info() {
	return mysql_get_host_info(conn);
}

int MySQL_Handler::escape_string(const char *src, char *&dest) {
	return mysql_escape_string(dest, src, strlen(src));
}

void MySQL_Handler::execute_query(struct mysql_query *&query) {
	int ping = this->ping();
	query->status = QUERY_STATUS_EXECUTED;
	if (!ping) {
		if (!mysql_query(conn, query->query)) {
			query->result = mysql_store_result(conn);
			query->last_row_idx = 0;
			query->insert_id = (int) mysql_insert_id(conn);
			query->affected_rows = (int) mysql_affected_rows(conn);
			if (query->result != NULL) {
				query->num_rows = (int) mysql_num_rows(query->result);
				query->num_fields = (int) mysql_num_fields(query->result);
				query->field_names.resize(query->num_fields);
				MYSQL_FIELD *field;
				for (int i = 0; field = mysql_fetch_field(query->result); ++i) {
					query->field_names[i] = (char*) malloc(sizeof(char) * (strlen(field->name) + 1));
					strcpy(query->field_names[i], field->name);
				}
				if (query->flags & QUERY_CACHED) {
					query->cache.resize(query->num_rows);
					for (int i = 0; i != query->num_rows; ++i) {
						MYSQL_ROW row = mysql_fetch_row(query->result);
						query->cache[i].resize(query->num_fields);
						for (int j = 0; j != query->num_fields; ++j) {
							if (row[j]) {
								query->cache[i][j] = (char*) malloc(sizeof(char) * (strlen(row[j]) + 1));
								strcpy(query->cache[i][j], row[j]);
							} else {
								query->cache[i][j] = (char*) malloc(sizeof(char) * 5); // NULL + \0
								strcpy(query->cache[i][j], "NULL");
							}
						}
					}
				} else {
					query->last_row = mysql_fetch_row(query->result);
				}
			}
		} else {
			query->error = get_errno();
			query->error_msg = get_error();
		}
	} else {
		query->error = ping;
	}
}

int MySQL_Handler::get_num_rows(struct mysql_query *query) {
	return query->num_rows;
}

int MySQL_Handler::get_num_fields(struct mysql_query *query) {
	return query->num_fields;
}

int MySQL_Handler::fetch_field(struct mysql_query *query, int fieldidx, char *&dest) {
	if ((0 <= fieldidx) && (fieldidx < query->num_fields)) {
		int len = strlen(query->field_names[fieldidx]) + 1;
		if (dest == NULL) {
			dest = (char*) malloc(sizeof(char) * len);
		}
		strcpy(dest, query->field_names[fieldidx]);
		return len;
	}
	return 0;
}

int MySQL_Handler::seek_row(struct mysql_query *query, int row) {
	if (row == -1) {
		row = query->last_row_idx + 1;
	}
	if (query->last_row_idx == row) {
		return 1;
	}
	if ((0 <= row) && (row < query->num_rows)) {
		if (!(query->flags & QUERY_CACHED)) {
			mysql_data_seek(query->result, row);
			query->last_row = mysql_fetch_row(query->result);
		}
		query->last_row_idx = row;
		return 1;
	}
	return 0;
}

int MySQL_Handler::fetch_num(struct mysql_query *query, int fieldidx, char *&dest) {
	if (query->flags & QUERY_CACHED) {
		if ((0 <= fieldidx) && (fieldidx < query->num_fields)) {
			int len = strlen(query->cache[query->last_row_idx][fieldidx]) + 1;
			if (dest == NULL) {
				dest = (char*) malloc(sizeof(char) * len);
			}
			strcpy(dest, query->cache[query->last_row_idx][fieldidx]);
			return len;
		}
	} else {
		if ((0 <= fieldidx) && (fieldidx < query->num_fields)) {
			if (query->last_row != NULL) {
				if (query->last_row[fieldidx]) {
					int len = strlen(query->last_row[fieldidx]) + 1;
					if (dest == NULL) {
						dest = (char*) malloc(sizeof(char) * len);
					}
					strcpy(dest, query->last_row[fieldidx]);
					return len;
				} else {
					if (dest == NULL) {
						dest = (char*) malloc(sizeof(char) * 5); // NULL\0
					}
					strcpy(dest, "NULL");
					return 5;
				}
			}
		}
	}
	return 0;
}

int MySQL_Handler::fetch_assoc(struct mysql_query *query, char *fieldname, char *&dest) {
	for (int i = 0, size = query->field_names.size(); i != size; ++i) {
		if (strcmp(query->field_names[i], fieldname) == 0) {
			return fetch_num(query, i, dest);
		}
	}
	return 0;
}