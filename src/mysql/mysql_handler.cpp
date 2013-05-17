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

#include "mysql.h"
 
#ifdef SQL_HANDLER_MYSQL

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

void MySQL_Handler::execute_query(SQL_Query *query) {
	MySQL_Query *q = dynamic_cast<MySQL_Query*>(query);
	if (q == 0) {
		return;
	}
	q->status = QUERY_STATUS_EXECUTED;
	if ((!ping()) && (!mysql_query(conn, q->query))) {
		q->error = 0;
		do {
			MySQL_Result *r = new MySQL_Result();
			r->result = mysql_store_result(conn);
			r->insert_id = mysql_insert_id(conn);
			r->affected_rows = mysql_affected_rows(conn);
			if (r->result != 0) {
				r->num_rows = mysql_num_rows(r->result);
				r->num_fields = mysql_num_fields(r->result);
				r->field_names.resize(r->num_fields);
				MYSQL_FIELD *field;
				for (int i = 0; field = mysql_fetch_field(r->result); ++i) {
					int len = strlen(field->name) + 1;
					r->field_names[i].first = (char*) malloc(sizeof(char) * len);
					strcpy(r->field_names[i].first, field->name);
					r->field_names[i].second = len;
				}
				if (query->flags & QUERY_CACHED) {
					r->cache.resize(r->num_rows);
					for (int i = 0; i != r->num_rows; ++i) {
						r->cache[i].resize(r->num_fields);
						MYSQL_ROW row = mysql_fetch_row(r->result);
						unsigned long *lengths = mysql_fetch_lengths(r->result);
						for (int j = 0; j != r->num_fields; ++j) {
							if (lengths[j]) {
								r->cache[i][j].first = (char*) malloc(sizeof(char) * (lengths[j] + 1));
								strcpy(r->cache[i][j].first, row[j]);
								r->cache[i][j].second = lengths[j] + 1;
							} else {
								r->cache[i][j].first = (char*) malloc(sizeof(char) * 5); // NULL + \0
								strcpy(r->cache[i][j].first, "NULL");
								r->cache[i][j].second = 5;
							}
						}
					}
				} else {
					r->last_row = mysql_fetch_row(r->result);
					r->last_row_lengths = mysql_fetch_lengths(r->result);
				}
			}
			query->results.push_back(r);
		} while (mysql_next_result(conn) == 0);
	} else {
		query->error = get_errno();
		query->error_msg = get_error();
	}
}

bool MySQL_Handler::seek_result(SQL_Query *query, int result) {
	if (result == -1) {
		result = query->last_result + 1;
	}
	if (query->last_result == result) {
		return true;
	}
	if ((0 <= result) && (result < query->results.size())) {
		query->last_result = result;
		return true;
	}
	return false;
}

bool MySQL_Handler::fetch_field(SQL_Query *query, int fieldidx, char *&dest, int &len) {
	SQL_Result *r = query->results[query->last_result];
	if ((0 <= fieldidx) && (fieldidx < r->num_fields)) {
		if (dest == 0) {
			dest = r->field_names[fieldidx].first;
			len = r->field_names[fieldidx].second;
			return false; // It is not a copy; we warn the user that he SHOULD NOT free dest.
		} else {
			strncpy(dest, r->field_names[fieldidx].first, len);
			return true;
		}
	}
	len = 0;
	return true;
}

bool MySQL_Handler::seek_row(SQL_Query *query, int row) {
	MySQL_Result *r = dynamic_cast<MySQL_Result*>(query->results[query->last_result]);
	if (r == 0) {
		return true;
	}
	if (row == -1) {
		row = r->last_row_idx + 1;
	}
	if (r->last_row_idx == row) {
		return true;
	}
	if ((0 <= row) && (row < r->num_rows)) {
		if (!(query->flags & QUERY_CACHED)) {
			mysql_data_seek(r->result, row);
			r->last_row = mysql_fetch_row(r->result);
			r->last_row_lengths = mysql_fetch_lengths(r->result);
		}
		r->last_row_idx = row;
		return true;
	}
	return false;
}

bool MySQL_Handler::fetch_num(SQL_Query *query, int fieldidx, char *&dest, int &len) {
	MySQL_Result *r = dynamic_cast<MySQL_Result*>(query->results[query->last_result]);
	if (r == 0) {
		len = 0;
		return true;
	}
	if ((r->num_rows != 0) && (0 <= fieldidx) && (fieldidx < r->num_fields)) {
		if (query->flags & QUERY_CACHED) {
			if (dest == 0) {
				len = r->cache[r->last_row_idx][fieldidx].second;
				dest = r->cache[r->last_row_idx][fieldidx].first;
				return false; // It is not a copy; we warn the user that he SHOULD NOT free dest.
			} else {
				memcpy(dest, r->cache[r->last_row_idx][fieldidx].first, len);
				return true;
			}
		} else {
			if (r->last_row != 0) {
				if (r->last_row_lengths[fieldidx]) {
					if (dest == 0) {
						len = r->last_row_lengths[fieldidx] + 1;
						dest = (char*) malloc(sizeof(char) * len);
						memcpy(dest, r->last_row[fieldidx], len);
					} else {
						memcpy(dest, r->last_row[fieldidx], len);
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

bool MySQL_Handler::fetch_assoc(SQL_Query *query, char *fieldname, char *&dest, int &len) {
	SQL_Result *r = query->results[query->last_result];
	for (int i = 0, size = r->field_names.size(); i != size; ++i) {
		if (strcmp(r->field_names[i].first, fieldname) == 0) {
			return fetch_num(query, i, dest, len);
		}
	}
	len = 0;
	return true;
}

#endif