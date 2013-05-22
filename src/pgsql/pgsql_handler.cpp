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

#include "pgsql.h"
 
#ifdef SQL_HANDLER_PGSQL

PgSQL_Handler::PgSQL_Handler() {
	handler_type = SQL_HANDLER_PGSQL;
	conn = NULL;
}

PgSQL_Handler::~PgSQL_Handler() {
	disconnect();
}

bool PgSQL_Handler::connect(const char *host, const char *user, const char *pass, const char *db, int port = 3306) {
	int len = snprintf(0, 0, "user=%s password=%s dbname=%s hostaddr=%s port=%d", user, pass, db, host, port) + 1;
	char *conninfo = (char*) malloc(len);
	snprintf(conninfo, len, "user=%s password=%s dbname=%s hostaddr=%s port=%d", user, pass, db, host, port);
	conn = PQconnectdb(conninfo);
	free(conninfo);
	return PQstatus(conn) == CONNECTION_OK;
}

void PgSQL_Handler::disconnect() {
	PQfinish(conn);
}

int PgSQL_Handler::get_errno() {
	return ping(); // TODO: Implement it properly.
}

const char *PgSQL_Handler::get_error() {
	return PQerrorMessage(conn);
}

int PgSQL_Handler::ping() {
	return PQstatus(conn);
}

const char *PgSQL_Handler::get_stat() {
	return 0; // TODO: Find an equivalent.
}

const char *PgSQL_Handler::get_charset() {
	return pg_encoding_to_char(PQclientEncoding(conn));
}

bool PgSQL_Handler::set_charset(char *charset) {
	return PQsetClientEncoding(conn, charset) == 0;
}

int PgSQL_Handler::escape_string(const char *src, char *&dest) {
	return PQescapeStringConn(conn, dest, src, strlen(src), 0);
}

void PgSQL_Handler::execute_query(SQL_Query *query) {
	PgSQL_Query *q = dynamic_cast<PgSQL_Query*>(query);
	if (q == 0) {
		return;
	}
	q->status = QUERY_STATUS_EXECUTED;
	if (!ping()) {
		PgSQL_Result *r = new PgSQL_Result();
		r->result = PQexec(conn, query->query);
		switch (PQresultStatus(r->result)) {
			case PGRES_EMPTY_QUERY: 
				break;
			case PGRES_NONFATAL_ERROR:
			case PGRES_FATAL_ERROR:
				q->error = atoi(PQresStatus(PQresultStatus(r->result)));
				q->error_msg = PQresultErrorMessage(r->result);
				break;
			case PGRES_TUPLES_OK:
				r->num_rows = PQntuples(r->result);
				r->num_fields = PQnfields(r->result);
				r->field_names.resize(r->num_fields);
				for (int i = 0; i != r->num_fields; ++i) {
					int len = strlen(PQfname(r->result, i)) + 1;
					r->field_names[i].first = (char*) malloc(sizeof(char) * len);
					strcpy(r->field_names[i].first, PQfname(r->result, i));
					r->field_names[i].second = len;
				}
				if (query->flags & QUERY_CACHED) {
					r->cache.resize(r->num_rows);
					for (int i = 0; i != r->num_rows; ++i) {
						r->cache[i].resize(r->num_fields);
						for (int j = 0; j != r->num_fields; ++j) {
							char *cell = PQgetvalue(r->result, i, j);
							int len = strlen(cell);
							if (len) {
								r->cache[i][j].first = (char*) malloc(sizeof(char) * (len + 1));
								strcpy(r->cache[i][j].first, cell);
								r->cache[i][j].second = len + 1;
							} else {
								r->cache[i][j].first = (char*) malloc(sizeof(char) * 5); // NULL + \0
								strcpy(r->cache[i][j].first, "NULL");
								r->cache[i][j].second = 5;
							}
						}
					}
				}
			case PGRES_COMMAND_OK:
				r->insert_id = PQoidValue(r->result);
				r->affected_rows = atoi(PQcmdTuples(r->result));
				break;
		}
		query->results.push_back(r);
	} else {
		query->error = get_errno();
		query->error_msg = get_error();
	}
}

bool PgSQL_Handler::seek_result(SQL_Query *query, int result) {
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

bool PgSQL_Handler::fetch_field(SQL_Query *query, int fieldidx, char *&dest, int &len) {
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

bool PgSQL_Handler::seek_row(SQL_Query *query, int row) {
	SQL_Result *r = query->results[query->last_result];
	if (row == -1) {
		row = r->last_row_idx + 1;
	}
	if (r->last_row_idx == row) {
		return true;
	}
	if ((0 <= row) && (row < r->num_rows)) {
		r->last_row_idx = row;
		return true;
	}
	return false;
}

bool PgSQL_Handler::fetch_num(SQL_Query *query, int fieldidx, char *&dest, int &len) {
	PgSQL_Result *r = dynamic_cast<PgSQL_Result*>(query->results[query->last_result]);
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
			int _len = strlen(PQgetvalue(r->result, r->last_row_idx, fieldidx));
			if (_len) {
				if (dest == 0) {
					len = _len + 1;
					dest = (char*) malloc(sizeof(char) * len);
					memcpy(dest, PQgetvalue(r->result, r->last_row_idx, fieldidx), len);
				} else {
					memcpy(dest, PQgetvalue(r->result, r->last_row_idx, fieldidx), len);
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
	len = 0;
	return true;
}

bool PgSQL_Handler::fetch_assoc(SQL_Query *query, char *fieldname, char *&dest, int &len) {
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