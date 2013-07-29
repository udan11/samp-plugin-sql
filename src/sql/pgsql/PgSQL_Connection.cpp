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

#include "../../log.h"

#include "PgSQL_Connection.h"
 
#ifdef PLUGIN_SUPPORTS_PGSQL

	#include "PgSQL_ResultSet.h"
	#include "PgSQL_Statement.h"

	PgSQL_Connection::PgSQL_Connection(int id, AMX *amx) : SQL_Connection(id, amx) {
		type = PLUGIN_SUPPORTS_PGSQL;
		conn = NULL;
		if (!PQisthreadsafe()) {
			log(LOG_WARNING, "libpq is not thread-safe! Crashes may occur!");
		}
	}

	PgSQL_Connection::~PgSQL_Connection() {
		disconnect();
	}

	bool PgSQL_Connection::connect(const char *host, const char *user, const char *pass, const char *db, int port) {
		if (port == 0) {
			port = PGSQL_DEFAULT_PORT;
		}
		int len = snprintf(NULL, 0, "user=%s password=%s dbname=%s hostaddr=%s port=%d", user, pass, db, host, port) + 1;
		char *conninfo = (char*) malloc(len);
		snprintf(conninfo, len, "user=%s password=%s dbname=%s hostaddr=%s port=%d", user, pass, db, host, port);
		conn = PQconnectdb(conninfo);
		free(conninfo);
		return PQstatus(conn) == CONNECTION_OK;
	}

	void PgSQL_Connection::disconnect() {
		PQfinish(conn);
	}

	int PgSQL_Connection::getErrorId() {
		return ping(); // TODO: Implement it properly.
	}

	const char *PgSQL_Connection::getError() {
		return PQerrorMessage(conn);
	}

	int PgSQL_Connection::ping() {
		return PQstatus(conn);
	}

	const char *PgSQL_Connection::getStat() {
		return 0; // TODO: Find an equivalent.
	}

	const char *PgSQL_Connection::getCharset() {
		return pg_encoding_to_char(PQclientEncoding(conn));
	}

	bool PgSQL_Connection::setCharset(char *charset) {
		return PQsetClientEncoding(conn, charset) == 0;
	}

	int PgSQL_Connection::escapeString(const char *src, char *&dest) {
		return PQescapeStringConn(conn, dest, src, strlen(src), 0);
	}

	void PgSQL_Connection::executeStatement(SQL_Statement *stmt) {
		PgSQL_Statement *q = static_cast<PgSQL_Statement*>(stmt);
		if (!ping()) {
			PgSQL_ResultSet *r = new PgSQL_ResultSet();
			r->result = PQexec(conn, stmt->query);
			switch (PQresultStatus(r->result)) {
				case PGRES_EMPTY_QUERY: 
					break;
				case PGRES_NONFATAL_ERROR:
				case PGRES_FATAL_ERROR:
					q->error = atoi(PQresStatus(PQresultStatus(r->result)));
					q->errorMsg = PQresultErrorMessage(r->result);
					break;
				case PGRES_TUPLES_OK:
					r->numRows = PQntuples(r->result);
					r->numFields = PQnfields(r->result);
					r->fieldNames.resize(r->numFields);
					for (int i = 0; i != r->numFields; ++i) {
						int len = strlen(PQfname(r->result, i)) + 1;
						r->fieldNames[i].first = (char*) malloc(sizeof(char) * len);
						strcpy(r->fieldNames[i].first, PQfname(r->result, i));
						r->fieldNames[i].second = len;
					}
					if (stmt->flags & STATEMENT_FLAGS_CACHED) {
						r->cache.resize(r->numRows);
						for (int i = 0; i != r->numRows; ++i) {
							r->cache[i].resize(r->numFields);
							for (int j = 0; j != r->numFields; ++j) {
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
					r->insertId = PQoidValue(r->result);
					r->affectedRows = atoi(PQcmdTuples(r->result));
					break;
			}
			stmt->resultSets.push_back(r);
		} else {
			stmt->error = getErrorId();
			stmt->errorMsg = getError();
		}
		q->status = STATEMENT_STATUS_EXECUTED;
	}

	bool PgSQL_Connection::seekResult(SQL_Statement *stmt, int resultIdx) {
		if (resultIdx == -1) {
			resultIdx = stmt->lastResultIdx + 1;
		}
		if (stmt->lastResultIdx == resultIdx) {
			return true;
		}
		if ((0 <= resultIdx) && (resultIdx < stmt->resultSets.size())) {
			stmt->lastResultIdx = resultIdx;
			return true;
		}
		return false;
	}

	bool PgSQL_Connection::fetchField(SQL_Statement *stmt, int fieldIdx, char *&dest, int &len) {
		SQL_ResultSet *r = stmt->resultSets[stmt->lastResultIdx];
		if ((0 <= fieldIdx) && (fieldIdx < r->numFields)) {
			if (dest == NULL) {
				dest = r->fieldNames[fieldIdx].first;
				len = r->fieldNames[fieldIdx].second;
				return false; // It is not a copy; we warn the user that he SHOULD NOT free dest.
			} else {
				strncpy(dest, r->fieldNames[fieldIdx].first, len);
				return true;
			}
		}
		len = 0;
		return true;
	}

	bool PgSQL_Connection::seekRow(SQL_Statement *stmt, int rowIdx) {
		SQL_ResultSet *r = stmt->resultSets[stmt->lastResultIdx];
		if (rowIdx < 0) {
			rowIdx = r->lastRowIdx - rowIdx;
		}
		if (r->lastRowIdx == rowIdx) {
			return true;
		}
		if ((0 <= rowIdx) && (rowIdx < r->numRows)) {
			r->lastRowIdx = rowIdx;
			return true;
		}
		return false;
	}

	bool PgSQL_Connection::fetchNum(SQL_Statement *stmt, int fieldIdx, char *&dest, int &len) {
		PgSQL_ResultSet *r = static_cast<PgSQL_ResultSet*>(stmt->resultSets[stmt->lastResultIdx]);
		if ((r->numRows != 0) && (0 <= fieldIdx) && (fieldIdx < r->numFields)) {
			if (stmt->flags & STATEMENT_FLAGS_CACHED) {
				if (dest == NULL) {
					len = r->cache[r->lastRowIdx][fieldIdx].second;
					dest = r->cache[r->lastRowIdx][fieldIdx].first;
					return false; // It is not a copy; we warn the user that he SHOULD NOT free dest.
				} else {
					memcpy(dest, r->cache[r->lastRowIdx][fieldIdx].first, len);
					return true;
				}
			} else {
				int _len = strlen(PQgetvalue(r->result, r->lastRowIdx, fieldIdx));
				if (_len) {
					if (dest == NULL) {
						len = _len + 1;
						dest = (char*) malloc(sizeof(char) * len);
						memcpy(dest, PQgetvalue(r->result, r->lastRowIdx, fieldIdx), len);
					} else {
						memcpy(dest, PQgetvalue(r->result, r->lastRowIdx, fieldIdx), len);
					}
				} else {
					if (dest == NULL) {
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

	bool PgSQL_Connection::fetchAssoc(SQL_Statement *stmt, char *fieldName, char *&dest, int &len) {
		SQL_ResultSet *r = stmt->resultSets[stmt->lastResultIdx];
		for (int i = 0, size = r->fieldNames.size(); i != size; ++i) {
			if (strcmp(r->fieldNames[i].first, fieldName) == 0) {
				return fetchNum(stmt, i, dest, len);
			}
		}
		len = 0;
		return true;
	}

#endif
