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

#include "pgsql_handler.h"

PgSQL_Handler::PgSQL_Handler() {
	conn = NULL;
}

PgSQL_Handler::~PgSQL_Handler() {
	disconnect();
}

bool PgSQL_Handler::connect(const char *host, const char *user, const char *pass, const char *db, int port = 5432) {
	int len = snprintf(0, 0, "user=%s password=%s dbname=%s hostaddr=%s port=%d", user, pass, db, host, port);
	char *conninfo = (char*) malloc(len + 1);
	conn = PQconnectdb(conninfo);
	free(conninfo);
	return PQstatus(conn) == CONNECTION_OK;
}

void PgSQL_Handler::disconnect() {
	PQfinish(conn);
}

int PgSQL_Handler::get_errno() {
	// TODO
	return 0;
}

const char *PgSQL_Handler::get_error() {
	return PQerrorMessage(conn);
}

int PgSQL_Handler::ping() {
	// TODO: return PQping(conn);
	return 0;
}

const char *PgSQL_Handler::get_stat() {
	// TODO
	return 0;
}

const char *PgSQL_Handler::get_charset() {
	// TODO
	return 0;
}

bool PgSQL_Handler::set_charset(char *charset) {
	// TODO
	return false;
}

int PgSQL_Handler::escape_string(const char *src, char *&dest) {
	return PQescapeStringConn(conn, dest, src, strlen(src), 0);
}

void PgSQL_Handler::execute_query(class SQL_Query *query) {
	PgSQL_Query *q = dynamic_cast<PgSQL_Query*>(query);
	if (q == 0) {
		log(LOG_ERROR, "PgSQL_Handler::execute_query: Invalid dynamic cast.");
		return;
	}
	int status = PQstatus(conn);
	if (status == CONNECTION_OK) {
		q->status = QUERY_STATUS_EXECUTED;
		q->result = PQexec(conn, query->query);
		if (PQresultStatus(q->result) == PGRES_COMMAND_OK) {
			q->error = 0;
			q->last_row_idx = 0;
			q->insert_id = PQoidValue(q->result);
			q->affected_rows = 0;
			if (strlen(PQcmdTuples(q->result))) {
				q->affected_rows = atoi(PQcmdTuples(q->result));
			}
			q->num_rows = 0;
			q->num_fields = 0;
			if (PQresultStatus(q->result) == PGRES_TUPLES_OK) {
				q->num_rows = PQntuples(q->result);
				q->num_fields = PQnfields(q->result);
				q->field_names.resize(q->num_fields);
				MYSQL_FIELD *field;
				for (int i = 0; i != q->num_fields; ++i) {
					int len = strlen(PQfname(q->result, i)) + 1;
					q->field_names[i].first = (char*) malloc(sizeof(char) * len);
					strcpy(q->field_names[i].first, PQfname(q->result, i));
					q->field_names[i].second = len;
				}
				if (q->flags & QUERY_CACHED) {
					q->cache.resize(q->num_rows);
					for (int i = 0; i != q->num_rows; ++i) {
						q->cache[i].resize(q->num_fields);
						for (int j = 0; j != q->num_fields; ++j) {
							char *cell = PQgetvalue(q->result, i, j);
							int len = strlen(cell);
							if (len) {
								q->cache[i][j].first = (char*) malloc(sizeof(char) * (len + 1));
								strcpy(q->cache[i][j].first, cell);
								q->cache[i][j].second = len + 1;
							} else {
								q->cache[i][j].first = (char*) malloc(sizeof(char) * 5); // NULL + \0
								strcpy(q->cache[i][j].first, "NULL");
								q->cache[i][j].second = 5;
							}
						}
					}
				}
			}
		} else {
			q->error = get_errno();
			q->error_msg = get_error();
		}
	}
}

bool PgSQL_Handler::next_result() {
	// TODO.
	return false;
}

void PgSQL_Handler::handle_result(class SQL_Query *query) {
	// TODO.
}

bool PgSQL_Handler::fetch_field(class SQL_Query *query, int fieldidx, char *&dest, int &len) {
	// TODO
	return false;
}

bool PgSQL_Handler::seek_row(class SQL_Query *query, int row) {
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
		q->last_row_idx = row;
		return true;
	}
	return false;
}

bool PgSQL_Handler::fetch_num(class SQL_Query *query, int fieldidx, char *&dest, int &len) {
	// TODO
	return false;
}

bool PgSQL_Handler::fetch_assoc(class SQL_Query *query, char *fieldname, char *&dest, int &len) {
	// TODO
	return false;
}