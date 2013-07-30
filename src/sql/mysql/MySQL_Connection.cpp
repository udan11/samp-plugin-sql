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

#include "MySQL_Connection.h"
 
#ifdef PLUGIN_SUPPORTS_MYSQL

	#include "MySQL_ResultSet.h"
	#include "MySQL_Statement.h"

	MySQL_Connection::MySQL_Connection(int id, AMX *amx) : SQL_Connection(id, amx) {
		type = PLUGIN_SUPPORTS_MYSQL;
		mutex = new Mutex();
		conn = mysql_init(NULL);
		my_bool reconnect = true;
		mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
	}

	MySQL_Connection::~MySQL_Connection() {
		disconnect();
		delete mutex;
	}

	bool MySQL_Connection::connect(const char *host, const char *user, const char *pass, const char *db, int port) {
		if (port == 0) {
			port = MYSQL_DEFAULT_PORT;
		}
		return mysql_real_connect(conn, host, user, pass, db, port, NULL, CLIENT_MULTI_STATEMENTS) ? true : false;
	}

	void MySQL_Connection::disconnect() {
		mysql_close(conn);
	}

	int MySQL_Connection::getErrorId() {
		return mysql_errno(conn);
	}

	const char *MySQL_Connection::getError() {
		return mysql_error(conn);
	}

	int MySQL_Connection::ping() {
		return mysql_ping(conn);
	}

	const char *MySQL_Connection::getStat() {
		return mysql_stat(conn);
	}

	const char *MySQL_Connection::getCharset() {
		return mysql_character_set_name(conn);
	}

	bool MySQL_Connection::setCharset(char *charset) {
		return mysql_set_character_set(conn, charset) == 0 ? true : false;
	}

	int MySQL_Connection::escapeString(const char *src, char *&dest) {
		return mysql_real_escape_string(conn, dest, src, strlen(src));
	}

	void MySQL_Connection::executeStatement(SQL_Statement *stmt) {
		mutex->lock();
		if ((!ping()) && (!mysql_query(conn, stmt->query))) {
			stmt->error = 0;
			do {
				MySQL_ResultSet *r = new MySQL_ResultSet();
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
					if (stmt->flags & STATEMENT_FLAGS_CACHED) {
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
				stmt->resultSets.push_back(r);
			} while (mysql_next_result(conn) == 0);
		} else {
			stmt->error = getErrorId();
			stmt->errorMsg = getError();
		}
		stmt->status = STATEMENT_STATUS_EXECUTED;
		mutex->unlock();
	}

	bool MySQL_Connection::seekResult(SQL_Statement *stmt, int resultIdx) {
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

	bool MySQL_Connection::fetchField(SQL_Statement *stmt, int fieldIdx, char *&dest, int &len) {
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

	bool MySQL_Connection::seekRow(SQL_Statement *stmt, int rowIdx) {
		MySQL_ResultSet *r = static_cast<MySQL_ResultSet*>(stmt->resultSets[stmt->lastResultIdx]);
		if (rowIdx < 0) {
			rowIdx = r->lastRowIdx - rowIdx;
		}
		if (r->lastRowIdx == rowIdx) {
			return true;
		}
		if ((0 <= rowIdx) && (rowIdx < r->numRows)) {
			if (!(stmt->flags & STATEMENT_FLAGS_CACHED)) {
				mysql_data_seek(r->result, rowIdx);
				r->lastRow = mysql_fetch_row(r->result);
				r->lastRowLens = mysql_fetch_lengths(r->result);
			}
			r->lastRowIdx = rowIdx;
			return true;
		}
		return false;
	}

	bool MySQL_Connection::fetchNum(SQL_Statement *stmt, int fieldIdx, char *&dest, int &len) {
		MySQL_ResultSet *r = static_cast<MySQL_ResultSet*>(stmt->resultSets[stmt->lastResultIdx]);
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
				if (r->lastRow != NULL) {
					if (r->lastRowLens[fieldIdx]) {
						if (dest == 0) {
							len = r->lastRowLens[fieldIdx] + 1;
							dest = (char*) malloc(sizeof(char) * len);
							memcpy(dest, r->lastRow[fieldIdx], len);
						} else {
							memcpy(dest, r->lastRow[fieldIdx], len);
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

	bool MySQL_Connection::fetchAssoc(SQL_Statement *stmt, char *fieldName, char *&dest, int &len) {
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
