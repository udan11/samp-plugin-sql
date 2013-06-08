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

MySQL_Handler::MySQL_Handler(int id, AMX *amx) : SQL_Handler(id, amx) {
	type = SQL_HANDLER_MYSQL;
	mutex = new Mutex();
	conn = mysql_init(NULL);
	my_bool reconnect = true;
	mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
}

MySQL_Handler::~MySQL_Handler() {
	disconnect();
	delete mutex;
}

bool MySQL_Handler::connect(const char *host, const char *user, const char *pass, const char *db, int port) {
	if (port == 0) {
		port = MYSQL_DEFAULT_PORT;
	}
	return mysql_real_connect(conn, host, user, pass, db, port, NULL, CLIENT_MULTI_STATEMENTS) ? true : false;
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
	if (q == NULL) {
		return;
	}
	mutex->lock();
	if ((!ping()) && (!mysql_query(conn, q->query))) {
		q->error = 0;
		do {
			MySQL_Result *r = new MySQL_Result();
			r->result = mysql_store_result(conn);
			r->insertId = mysql_insert_id(conn);
			r->affectedRows = mysql_affected_rows(conn);
			if (r->result != NULL) {
				r->numRows = mysql_num_rows(r->result);
				r->numFields = mysql_num_fields(r->result);
				r->fieldNames.resize(r->numFields);
				MYSQL_FIELD *field;
				for (int i = 0; field = mysql_fetch_field(r->result); ++i) {
					int len = strlen(field->name) + 1;
					r->fieldNames[i].first = (char*) malloc(sizeof(char) * len);
					strcpy(r->fieldNames[i].first, field->name);
					r->fieldNames[i].second = len;
				}
				if (query->flags & QUERY_CACHED) {
					r->cache.resize(r->numRows);
					for (int i = 0; i != r->numRows; ++i) {
						r->cache[i].resize(r->numFields);
						MYSQL_ROW row = mysql_fetch_row(r->result);
						unsigned long *lengths = mysql_fetch_lengths(r->result);
						for (int j = 0; j != r->numFields; ++j) {
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
					r->lastRow = mysql_fetch_row(r->result);
					r->lastRowLens = mysql_fetch_lengths(r->result);
				}
			}
			query->results.push_back(r);
		} while (mysql_next_result(conn) == 0);
	} else {
		query->error = get_errno();
		query->errorMsg = get_error();
	}
	mutex->unlock();
	q->status = QUERY_STATUS_EXECUTED;
}

bool MySQL_Handler::seek_result(SQL_Query *query, int result) {
	if (result == -1) {
		result = query->lastResultIdx + 1;
	}
	if (query->lastResultIdx == result) {
		return true;
	}
	if ((0 <= result) && (result < query->results.size())) {
		query->lastResultIdx = result;
		return true;
	}
	return false;
}

bool MySQL_Handler::fetch_field(SQL_Query *query, int fieldidx, char *&dest, int &len) {
	SQL_Result *r = query->results[query->lastResultIdx];
	if ((0 <= fieldidx) && (fieldidx < r->numFields)) {
		if (dest == NULL) {
			dest = r->fieldNames[fieldidx].first;
			len = r->fieldNames[fieldidx].second;
			return false; // It is not a copy; we warn the user that he SHOULD NOT free dest.
		} else {
			strncpy(dest, r->fieldNames[fieldidx].first, len);
			return true;
		}
	}
	len = 0;
	return true;
}

bool MySQL_Handler::seek_row(SQL_Query *query, int row) {
	MySQL_Result *r = dynamic_cast<MySQL_Result*>(query->results[query->lastResultIdx]);
	if (row < 0) {
		row = r->lastRowIdx - row;
	}
	if (r->lastRowIdx == row) {
		return true;
	}
	if ((0 <= row) && (row < r->numRows)) {
		if (!(query->flags & QUERY_CACHED)) {
			mysql_data_seek(r->result, row);
			r->lastRow = mysql_fetch_row(r->result);
			r->lastRowLens = mysql_fetch_lengths(r->result);
		}
		r->lastRowIdx = row;
		return true;
	}
	return false;
}

bool MySQL_Handler::fetch_num(SQL_Query *query, int fieldidx, char *&dest, int &len) {
	MySQL_Result *r = dynamic_cast<MySQL_Result*>(query->results[query->lastResultIdx]);
	if (r == NULL) {
		len = 0;
		return true;
	}
	if ((r->numRows != 0) && (0 <= fieldidx) && (fieldidx < r->numFields)) {
		if (query->flags & QUERY_CACHED) {
			if (dest == NULL) {
				len = r->cache[r->lastRowIdx][fieldidx].second;
				dest = r->cache[r->lastRowIdx][fieldidx].first;
				return false; // It is not a copy; we warn the user that he SHOULD NOT free dest.
			} else {
				memcpy(dest, r->cache[r->lastRowIdx][fieldidx].first, len);
				return true;
			}
		} else {
			if (r->lastRow != NULL) {
				if (r->lastRowLens[fieldidx]) {
					if (dest == 0) {
						len = r->lastRowLens[fieldidx] + 1;
						dest = (char*) malloc(sizeof(char) * len);
						memcpy(dest, r->lastRow[fieldidx], len);
					} else {
						memcpy(dest, r->lastRow[fieldidx], len);
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
	SQL_Result *r = query->results[query->lastResultIdx];
	for (int i = 0, size = r->fieldNames.size(); i != size; ++i) {
		if (strcmp(r->fieldNames[i].first, fieldname) == 0) {
			return fetch_num(query, i, dest, len);
		}
	}
	len = 0;
	return true;
}

#endif
