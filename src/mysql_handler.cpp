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

#include "mysql_handler.h"

MySQL_Handler::MySQL_Handler() {
	amx = NULL;
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

const char *MySQL_Handler::get_error() {
	return mysql_error(conn);
}

int MySQL_Handler::ping() {
	return mysql_ping(conn);
}

const char *MySQL_Handler::get_stat() {
	return mysql_stat(conn);
}

const char *MySQL_Handler::get_charset() {
	return mysql_character_set_name(conn);
}

bool MySQL_Handler::set_charset(char *charset) {
	return mysql_set_character_set(conn, charset) == 0 ? true : false;
}

int MySQL_Handler::escape_string(const char *src, char *&dest) {
	return mysql_real_escape_string(conn, dest, src, strlen(src));
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
			query->num_rows = 0;
			query->num_fields = 0;
			if (query->result != NULL) {
				query->num_rows = (int) mysql_num_rows(query->result);
				query->num_fields = (int) mysql_num_fields(query->result);
				query->field_names.resize(query->num_fields);
				MYSQL_FIELD *field;
				for (int i = 0; field = mysql_fetch_field(query->result); ++i) {
					int len = strlen(field->name) + 1;
					query->field_names[i].first = (char*) malloc(sizeof(char) * len);
					strcpy(query->field_names[i].first, field->name);
					query->field_names[i].second = len;
				}
				if (query->flags & QUERY_CACHED) {
					query->cache.resize(query->num_rows);
					for (int i = 0; i != query->num_rows; ++i) {
						query->cache[i].resize(query->num_fields);
						MYSQL_ROW row = mysql_fetch_row(query->result);
						unsigned long *lengths = mysql_fetch_lengths(query->result);
						for (int j = 0; j != query->num_fields; ++j) {
							if (lengths[j]) {
								query->cache[i][j].first = (char*) malloc(sizeof(char) * (lengths[j] + 1));
								strcpy(query->cache[i][j].first, row[j]);
								query->cache[i][j].second = lengths[j] + 1;
							} else {
								query->cache[i][j].first = (char*) malloc(sizeof(char) * 5); // NULL + \0
								strcpy(query->cache[i][j].first, "NULL");
								query->cache[i][j].second = 5;
							}
						}
					}
				} else {
					query->last_row = mysql_fetch_row(query->result);
					query->last_row_lengths = mysql_fetch_lengths(query->result);
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

bool MySQL_Handler::fetch_field(struct mysql_query *query, int fieldidx, char *&dest, int &len) {
	if ((0 <= fieldidx) && (fieldidx < query->num_fields)) {
		if (dest == NULL) {
			dest = query->field_names[fieldidx].first;
			len = query->field_names[fieldidx].second;
			return false; // It is not a copy; we warn the user that he SHOULD NOT free dest.
		} else {
			strncpy(dest, query->field_names[fieldidx].first, len);
			return true;
		}
	}
	len = 0;
	return true;
}

bool MySQL_Handler::seek_row(struct mysql_query *query, int row) {
	if (row == -1) {
		row = query->last_row_idx + 1;
	}
	if (query->last_row_idx == row) {
		return true;
	}
	if ((0 <= row) && (row < query->num_rows)) {
		if (!(query->flags & QUERY_CACHED)) {
			mysql_data_seek(query->result, row);
			query->last_row = mysql_fetch_row(query->result);
			query->last_row_lengths = mysql_fetch_lengths(query->result);
		}
		query->last_row_idx = row;
		return true;
	}
	return false;
}

bool MySQL_Handler::fetch_num(struct mysql_query *query, int fieldidx, char *&dest, int &len) {
	if ((query->num_rows != 0) && (0 <= fieldidx) && (fieldidx < query->num_fields)) {
		if (query->flags & QUERY_CACHED) {
			if (dest == NULL) {
				len = query->cache[query->last_row_idx][fieldidx].second;
				dest = query->cache[query->last_row_idx][fieldidx].first;
				return false; // It is not a copy; we warn the user that he SHOULD NOT free dest.
			} else {
				memcpy(dest, query->cache[query->last_row_idx][fieldidx].first, len);
				return true;
			}
		} else {
			if (query->last_row != NULL) {
				if (query->last_row_lengths[fieldidx]) {
					if (dest == NULL) {
						len = query->last_row_lengths[fieldidx] + 1;
						dest = (char*) malloc(sizeof(char) * len);
						memcpy(dest, query->last_row[fieldidx], len);
					} else {
						memcpy(dest, query->last_row[fieldidx], len);
					}
				} else {
					if (dest == NULL) {
						len = 5; // NULL\0
						dest = (char*) malloc(sizeof(char) * 5);
						strcpy(dest, "NULL");
					} else {
						strncpy(dest, "NULL", len);
					}
				}
				return true;
			}
		}
	}
	len = 0;
	return true;
}

bool MySQL_Handler::fetch_assoc(struct mysql_query *query, char *fieldname, char *&dest, int &len) {
	for (int i = 0, size = query->field_names.size(); i != size; ++i) {
		if (strcmp(query->field_names[i].first, fieldname) == 0) {
			return fetch_num(query, i, dest, len);
		}
	}
	len = 0;
	return true;
}