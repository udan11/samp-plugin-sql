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

#include "mysql_handler.h"

MySQL_Handler::MySQL_Handler() {
	handler_type = SQL_HANDLER_MYSQL;
	conn = mysql_init(0);
	bool reconnect = true;
	mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
}

MySQL_Handler::~MySQL_Handler() {
	disconnect();
}

bool MySQL_Handler::connect(const char *host, const char *user, const char *pass, const char *db, int port = 3306) {
	return mysql_real_connect(conn, host, user, pass, db, port, 0, CLIENT_MULTI_STATEMENTS) ? true : false;
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

void MySQL_Handler::execute_query(class SQL_Query *query) {
	MySQL_Query *q = dynamic_cast<MySQL_Query*>(query);
	if (q == 0) {
		log(LOG_ERROR, "MySQL_Handler::execute_query: Invalid dynamic cast.");
		return;
	}
	int ping = this->ping();
	if (!ping) {
		q->status = QUERY_STATUS_EXECUTED;
		if (!mysql_query(conn, q->query)) {
			q->error = 0;
			q->result = mysql_store_result(conn);
			q->last_row_idx = 0;
			q->insert_id = mysql_insert_id(conn);
			q->affected_rows = mysql_affected_rows(conn);
			q->num_rows = 0;
			q->num_fields = 0;
			if (q->result != 0) {
				q->num_rows = mysql_num_rows(q->result);
				q->num_fields = mysql_num_fields(q->result);
				q->field_names.resize(q->num_fields);
				MYSQL_FIELD *field;
				for (int i = 0; field = mysql_fetch_field(q->result); ++i) {
					int len = strlen(field->name) + 1;
					q->field_names[i].first = (char*) malloc(sizeof(char) * len);
					strcpy(q->field_names[i].first, field->name);
					q->field_names[i].second = len;
				}
				if (q->flags & QUERY_CACHED) {
					q->cache.resize(q->num_rows);
					for (int i = 0; i != q->num_rows; ++i) {
						q->cache[i].resize(q->num_fields);
						MYSQL_ROW row = mysql_fetch_row(q->result);
						unsigned long *lengths = mysql_fetch_lengths(q->result);
						for (int j = 0; j != q->num_fields; ++j) {
							if (lengths[j]) {
								q->cache[i][j].first = (char*) malloc(sizeof(char) * (lengths[j] + 1));
								strcpy(q->cache[i][j].first, row[j]);
								q->cache[i][j].second = lengths[j] + 1;
							} else {
								q->cache[i][j].first = (char*) malloc(sizeof(char) * 5); // NULL + \0
								strcpy(q->cache[i][j].first, "NULL");
								q->cache[i][j].second = 5;
							}
						}
					}
				} else {
					q->last_row = mysql_fetch_row(q->result);
					q->last_row_lengths = mysql_fetch_lengths(q->result);
				}
			}
		} else {
			q->error = get_errno();
			q->error_msg = get_error();
		}
	} else {
		q->error = ping;
	}
}

bool MySQL_Handler::next_result() {
	// TODO.
	return false;
}

void MySQL_Handler::handle_result(class SQL_Query *query) {
	// TODO.
}

bool MySQL_Handler::fetch_field(class SQL_Query *query, int fieldidx, char *&dest, int &len) {
	MySQL_Query *q = dynamic_cast<MySQL_Query*>(query);
	if (q == 0) {
		log(LOG_ERROR, "MySQL_Handler::fetch_field: Invalid dynamic cast.");
		return true;
	}
	if ((0 <= fieldidx) && (fieldidx < q->num_fields)) {
		if (dest == 0) {
			dest = q->field_names[fieldidx].first;
			len = q->field_names[fieldidx].second;
			return false; // It is not a copy; we warn the user that he SHOULD NOT free dest.
		} else {
			strncpy(dest, q->field_names[fieldidx].first, len);
			return true;
		}
	}
	len = 0;
	return true;
}

bool MySQL_Handler::seek_row(class SQL_Query *query, int row) {
	MySQL_Query *q = dynamic_cast<MySQL_Query*>(query);
	if (q == 0) {
		log(LOG_ERROR, "MySQL_Handler::seek_row: Invalid dynamic cast.");
		return true;
	}
	if (row == -1) {
		row = q->last_row_idx + 1;
	}
	if (q->last_row_idx == row) {
		return true;
	}
	if ((0 <= row) && (row < q->num_rows)) {
		if (!(q->flags & QUERY_CACHED)) {
			mysql_data_seek(q->result, row);
			q->last_row = mysql_fetch_row(q->result);
			q->last_row_lengths = mysql_fetch_lengths(q->result);
		}
		q->last_row_idx = row;
		return true;
	}
	return false;
}

bool MySQL_Handler::fetch_num(class SQL_Query *query, int fieldidx, char *&dest, int &len) {
	MySQL_Query *q = dynamic_cast<MySQL_Query*>(query);
	if (q == 0) {
		log(LOG_ERROR, "MySQL_Handler::fetch_num: Invalid dynamic cast.");
		return true;
	}
	if ((q->num_rows != 0) && (0 <= fieldidx) && (fieldidx < q->num_fields)) {
		if (q->flags & QUERY_CACHED) {
			if (dest == 0) {
				len = q->cache[q->last_row_idx][fieldidx].second;
				dest = q->cache[q->last_row_idx][fieldidx].first;
				return false; // It is not a copy; we warn the user that he SHOULD NOT free dest.
			} else {
				memcpy(dest, q->cache[q->last_row_idx][fieldidx].first, len);
				return true;
			}
		} else {
			if (q->last_row != 0) {
				if (q->last_row_lengths[fieldidx]) {
					if (dest == 0) {
						len = q->last_row_lengths[fieldidx] + 1;
						dest = (char*) malloc(sizeof(char) * len);
						memcpy(dest, q->last_row[fieldidx], len);
					} else {
						memcpy(dest, q->last_row[fieldidx], len);
					}
				} else {
					if (dest == 0) {
						len = 5; // NULL + \0
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

bool MySQL_Handler::fetch_assoc(class SQL_Query *query, char *fieldname, char *&dest, int &len) {
	MySQL_Query *q = dynamic_cast<MySQL_Query*>(query);
	if (q == 0) {
		log(LOG_ERROR, "MySQL_Handler::fetch_assoc: Invalid dynamic cast.");
		return true;
	}
	for (int i = 0, size = q->field_names.size(); i != size; ++i) {
		if (strcmp(q->field_names[i].first, fieldname) == 0) {
			return fetch_num(q, i, dest, len);
		}
	}
	len = 0;
	return true;
}