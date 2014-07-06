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

#include <cstdio>

#include "sdk/amx/amx.h"
#include "sdk/amx/amx2.h"

#include "sql/sql.h"
#include "sql/SQL_Connection.h"
#include "sql/SQL_Pools.h"
#include "sql/SQL_ResultSet.h"
#include "sql/SQL_Statement.h"

#include "Logger.h"

#include "Natives.h"

cell AMX_NATIVE_CALL Natives::sql_debug(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	Logger::fileLevel = params[1];
	Logger::consoleLevel = params[2];
	Logger::log(LOG_WARNING, "Natives::sql_debug: Switching the log levels to (%d, %d)...", Logger::fileLevel, Logger::consoleLevel);
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_connect(AMX *amx, cell *params) {
	if (params[0] < 6 * 4) {
		return 0;
	}
	SQL_Connection *conn = SQL_Pools::newConnection(amx, params[1]);
	if (conn == NULL) {
		Logger::log(LOG_ERROR, "Natives::sql_connect: Unknown SQL type (%d)!", params[1]);
		return 0;
	}
	int id = conn->id;
	char *host = NULL, *user = NULL, *pass = NULL, *db = NULL;
	amx_StrParam(amx, params[2], host);
	amx_StrParam(amx, params[3], user);
	amx_StrParam(amx, params[4], pass);
	amx_StrParam(amx, params[5], db);
	if (host == NULL) {
		Logger::log(LOG_WARNING, "Natives::sql_connect: The `host` field is empty.");
	}
	if (user == NULL) {
		Logger::log(LOG_WARNING, "Natives::sql_connect: The `user` field is empty.");
	}
	if (pass == NULL) {
		Logger::log(LOG_WARNING, "Natives::sql_connect: The `pass` field is empty.");
	}
	if (db == NULL) {
		Logger::log(LOG_WARNING, "Natives::sql_connect: The `db` field is empty.");
	}
	Logger::log(LOG_INFO, "Natives::sql_connect: Connecting to database (type = %d) %s:***@%s:%d/%s...", params[1], user, host, params[6], db);
	if (conn->connect(host, user, pass, db, params[6])) {
		Logger::log(LOG_INFO, "Natives::sql_connect: Connection (conn->id = %d) was succesful!", id);
		SQL_Pools::connections[id] = conn;
		conn->startWorker();
	} else {
		Logger::log(LOG_WARNING, "Natives::sql_connect: Connection (conn->id = %d) failed! (error = %d, %s)", id, conn->getErrorId(), conn->getError());
		delete conn;
		id = 0;
	}
	return id;
}

cell AMX_NATIVE_CALL Natives::sql_disconnect(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(params[1])) {
		return 0;
	}
	SQL_Connection *conn = SQL_Pools::connections[params[1]];
	SQL_Pools::connections.erase(params[1]);
	conn->stopWorker();
	delete conn;
	Logger::log(LOG_INFO, "Natives::sql_disconnect: Connection (conn->id = %d) was destroyed!", params[1]);
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_wait(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(params[1])) {
		return 0;
	}
	SQL_Connection *conn = SQL_Pools::connections[params[1]];
	while (!conn->pending.empty()) {
		SLEEP(WORKER_TICK_RATE);
	}
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_set_charset(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(params[1])) {
		return 0;
	}
	char *charset = NULL;
	amx_StrParam(amx, params[2], charset);
	if (charset == NULL) {
		Logger::log(LOG_WARNING, "Natives::sql_set_charset: New charset is null.");
		return false;
	}
	Logger::log(LOG_INFO, "Natives::sql_set_charset: Setting conn's charset (conn->id = %d) to %s.", params[1], charset);
	return SQL_Pools::connections[params[1]]->setCharset(charset);
}

cell AMX_NATIVE_CALL Natives::sql_get_charset(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(params[1])) {
		return 0;
	}
	char *tmp = (char*) SQL_Pools::connections[params[1]]->getCharset();
	Logger::log(LOG_DEBUG, "Natives::sql_get_charset: Getting conn's charset (conn->id = %d, conn->getCharset() = %s)...", params[1], tmp);
	int len = strlen(tmp);
	if (params[3] < 2) {
		Logger::log(LOG_DEBUG, "Natives::sql_get_charset: Specified destination size is smaller than 2, setting it to %d.", len);
		amx_SetCString(amx, params[2], tmp, len);
	} else {
		amx_SetCString(amx, params[2], tmp, params[3]);
	}
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_ping(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return -1;
	}
	if (!SQL_Pools::isValidConnection(params[1])) {
		return -1;
	}
	Logger::log(LOG_DEBUG, "Natives::sql_ping: Pinging conn (conn->id = %d)...", params[1]);
	return SQL_Pools::connections[params[1]]->ping();
}

cell AMX_NATIVE_CALL Natives::sql_get_stat(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(params[1])) {
		return 0;
	}
	char *tmp = (char*) SQL_Pools::connections[params[1]]->getStat();
	Logger::log(LOG_DEBUG, "Natives::sql_get_stat: Getting conn's statistics (conn->id = %d, conn->getStat() = %s)...", params[1], tmp);
	int len = strlen(tmp);
	if (params[3] < 2) {
		Logger::log(LOG_DEBUG, "Natives::sql_get_stat: Specified destination size is smaller than 2, setting it to %d.", len);
		amx_SetCString(amx, params[2], tmp, len);
	} else {
		amx_SetCString(amx, params[2], tmp, params[3]);
	}
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_escape_string(AMX *amx, cell *params) {
	if (params[0] < 3 * 4) {
		return -1;
	}
	char *src = NULL;
	amx_StrParam(amx, params[2], src);
	if (src == NULL) {
		Logger::log(LOG_DEBUG, "Natives::sql_escape_string: Escaping null string...");
		amx_SetCString(amx, params[3], "", params[4]);
		return 0;
	}
	if (!SQL_Pools::isValidConnection(params[1])) {
		return -1;
	}
	Logger::log(LOG_DEBUG, "Natives::sql_escape_string: Escaping (conn->id = %d) string '%s'...", params[1], src);
	char *dest = (char*) malloc(sizeof(char) * strlen(src) * 2); // *2 in case every character is escaped
	if (dest != NULL) {
		int len = SQL_Pools::connections[params[1]]->escapeString(src, dest);
		if (len != 0) {
			if (params[4] < 2) {
				Logger::log(LOG_DEBUG, "Natives::sql_escape_string: Specified destination size is smaller than 2, setting it to %d.", len);
				amx_SetCString(amx, params[3], dest, len);
			} else {
				amx_SetCString(amx, params[3], dest, params[4]);
			}
			free(dest);
		}
		return len;
	}
	return 0;
}

cell AMX_NATIVE_CALL Natives::sql_format(AMX *amx, cell *params) {
	// This method is basically a proxy for sprintf.
	if (params[0] < 4 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(params[1])) {
		Logger::log(LOG_WARNING, "Natives::sql_format: Invalid connection! (conn->id = %d)", params[1]);
		return 0;
	}
	char *format = NULL;
	amx_StrParam(amx, params[4], format);
	Logger::log(LOG_DEBUG, "Natives::sql_format: Formatting string %s using conn->id = %d.", format, params[1]);
	if (format == NULL) {
		amx_SetCString(amx, params[2], "", 1);
		return 0;
	}
	int dest_len = params[3];
	if (dest_len < 2) { // Probably a multi-dimensional array.
		dest_len = strlen(format) + (((params[0] / 4) - 4) * 128); // 128 bytes for each additional parameter.
	}
	char *output = (char*) malloc(dest_len * sizeof(char)), specifier[16];
	memset(output, 0, dest_len * sizeof(char));
	memset(specifier, 0, sizeof(specifier));
	for (int i = 0, len = strlen(format), inSpecifier = false, p = 4; i != len; ++i) {
		if (inSpecifier) {
			specifier[strlen(specifier)] = format[i];
			if ((isalpha(format[i])) || (format[i] == '%')) {
				// Specifier ended.
				inSpecifier = false;
				cell *ptr;
				char *tmp, *tmp2;
				switch (format[i]) {
					// Integers.
					case 'd':
					case 'i':
					case 'o':
					case 'x':
					case 'X':
					case 'c':
						amx_GetAddr(amx, params[++p], &ptr);
						tmp = (char*) malloc(64 * sizeof(char));
						sprintf(tmp, specifier, *ptr);
						strcat(output, tmp);
						free(tmp);
						break;
					// Floats.
					case 'f':
					case 'F':
					case 'e':
					case 'E':
					case 'g':
					case 'G':
					case 'a':
					case 'A':
						amx_GetAddr(amx, params[++p], &ptr);
						tmp = (char*) malloc(128 * sizeof(char));
						sprintf(tmp, specifier, amx_ctof(*ptr));
						strcat(output, tmp);
						free(tmp);
						break;					
					// Strings.
					case 's': 
						amx_StrParam(amx, params[++p], tmp);
						/*
						tmp2 = (char*) malloc(2 * sizeof(char) * strlen(tmp));
						sprintf(tmp2, specifier, tmp);
						strcat(output, tmp2);
						free(tmp2);
						*/
						strcat(output, tmp);
						break;
					// Others.
					case '%':
						strcat(output, "%");
						break;
					// Custom specifiers.
					case 'z': // %z escapes the string.
						amx_StrParam(amx, params[++p], tmp);
						tmp2 = (char*) malloc(2 * sizeof(char) * strlen(tmp));
						if (SQL_Pools::connections[params[1]]->escapeString(tmp, tmp2)) {
							/*
							specifier[strlen(specifier) - 1] = 's';
							sprintf(tmp2, specifier, tmp2);
							*/
							strcat(output, tmp2);
						}
						free(tmp2);
						break;
					default:
						Logger::log(LOG_WARNING, "Natives::sql_format: Unknown specifier: %s.", specifier);
						break;
				}
			}
		} else if (format[i] == '%') {
			// Specifier started.
			inSpecifier = true;
			memset(specifier, 0, sizeof(specifier));
			specifier[0] = '%';
		} else {
			output[strlen(output)] = format[i];
		}
	}
	amx_SetCString(amx, params[2], output, dest_len);
	int outputLen = strlen(output);
	free(output);
	return outputLen;
}

cell AMX_NATIVE_CALL Natives::sql_query(AMX *amx, cell *params) {
	if (params[0] < 5 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(params[1])) {
		Logger::log(LOG_WARNING, "Natives::sql_query: Invalid connection! (conn->id = %d)", params[1]);
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::newStatement(amx, params[1]);
	if (stmt == NULL) {
		Logger::log(LOG_WARNING, "Natives::sql_query: Invalid connection! (conn->id = %d, conn->type = %d)", params[1], SQL_Pools::connections[params[1]]->type);
		return 0;
	}
	int id = stmt->id;
	stmt->connectionId = params[1];
	amx_GetCString(amx, params[2], stmt->query);
	stmt->flags = params[3];
	amx_GetCString(amx, params[4], stmt->callback);
	amx_GetCString(amx, params[5], stmt->format);
	for (int i = 0, len = strlen(stmt->format), p = 6; i < len; ++i, ++p) {
		switch (stmt->format[i]) {
			case 'a':
			case 'A':
				cell *ptrArr, *ptrLen, *arr, len;
				amx_GetAddr(amx, params[p], &ptrArr);
				amx_GetAddr(amx, params[p + 1], &ptrLen);
				len = sizeof(cell) *(*ptrLen);
				arr = (cell*) malloc(len);
				if (arr != NULL) {
					memcpy(arr, ptrArr, len);
					stmt->paramsArr.push_back(std::make_pair(arr, *ptrLen));
				}
				break;
			case 'b':
			case 'B':
			case 'c':
			case 'C':
			case 'd':
			case 'D':
			case 'i':
			case 'I':
			case 'f': 
			case 'F':
				cell *ptr;
				amx_GetAddr(amx, params[p], &ptr);
				stmt->paramsC.push_back(*ptr);
				break;
			case 'r':
			case 'R':
				--p; // We didn't read any parameter.
				break;
			case 's':
			case 'S':
				char *str;
				amx_GetCString(amx, params[p], str);
				stmt->paramsStr.push_back(str);
				break;
			case '&': 
				stmt->paramsC.push_back(params[p]);
				++i; // Skipping next specifier (&x).
				break;
			default: 
				Logger::log(LOG_WARNING, "Natives::sql_query: Format '%c' is not recognized.", stmt->format[i]);
				break;
		}
	}
	SQL_Pools::statements[stmt->id] = stmt;
	SQL_Connection *conn = SQL_Pools::connections[stmt->connectionId];
	if (stmt->flags & STATEMENT_FLAGS_THREADED) {
		Logger::log(LOG_DEBUG, "Natives::sql_query: Scheduling statement (stmt->id = %d, stmt->query = %s, stmt->callback = %s) for execution...", stmt->id, stmt->query, stmt->callback);
		conn->pending.push(stmt);
	} else {
		Logger::log(LOG_DEBUG, "Natives::sql_query: Executing statement (stmt->id = %d, stmt->query = %s)...", stmt->id, stmt->query);
		conn->executeStatement(stmt);
		if ((strlen(stmt->callback)) || (stmt->error != 0)) {
			Logger::log(LOG_DEBUG, "Natives::sql_query: Executing statement callback (stmt->id = %d, stmt->error = %d, stmt->callback = %s)...", stmt->id, stmt->error, stmt->callback);
			stmt->executeCallback();
		} else {
			Logger::log(LOG_DEBUG, "Natives::sql_query: Statement executed (stmt->id = %d, stmt->error = %d). No callback found!", stmt->id, stmt->error);
		}
	}
	return id;
}

cell AMX_NATIVE_CALL Natives::sql_free_result(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	Logger::log(LOG_DEBUG, "Natives::sql_free_result: Freeing statement (stmt->id = %d)...", params[1]);
	SQL_Pools::statements.erase(params[1]);
	delete stmt;
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_store_result(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	Logger::log(LOG_DEBUG, "Natives::sql_store_result: Storing statement (stmt->id = %d)...", params[1]);
	// Switching the state of this statement to non-threaded (it has to be freed manually).
	stmt->flags &= ~STATEMENT_FLAGS_THREADED;
	stmt->status = STATEMENT_STATUS_EXECUTED;
	return 1;
}

cell AMX_NATIVE_CALL Natives::sql_insert_id(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	Logger::log(LOG_DEBUG, "Natives::sql_insert_id: Retrieving insert ID (stmt->id = %d)...", params[1]);
	return stmt->resultSets[stmt->lastResultIdx]->insertId;
}

cell AMX_NATIVE_CALL Natives::sql_affected_rows(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	Logger::log(LOG_DEBUG, "Natives::sql_affected_rows: Retrieving the count of affected rows (stmt->id = %d)...", params[1]);
	return stmt->resultSets[stmt->lastResultIdx]->affectedRows;
}

cell AMX_NATIVE_CALL Natives::sql_error(AMX *amx, cell *params) {
	if (params[0] < 1 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	Logger::log(LOG_DEBUG, "Natives::sql_error: Retrieving error code (stmt->id = %d)...", params[1]);
	return stmt->error;
}

cell AMX_NATIVE_CALL Natives::sql_error_string(AMX *amx, cell *params) {
	if (params[0] < 3 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(stmt->connectionId)) {
		return 0;
	}
	int len = strlen(stmt->errorMsg);
	if (len != 0) {
		char *error = (char*) malloc(sizeof(char) * len);
		strcpy(error, stmt->errorMsg);
		if (params[3] < 2) {
			Logger::log(LOG_DEBUG, "Natives::sql_error_string: Specified destination size is smaller than 2, setting it to %d.", len);
			amx_SetCString(amx, params[2], error, len);
		} else {
			amx_SetCString(amx, params[2], error, params[3]);
		}
	}
	Logger::log(LOG_DEBUG, "Natives::sql_error_string: Retrieving error string (stmt->id = %d)...", params[1]);
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_num_rows(AMX *amx, cell *params) {
	if (params[0] < 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	Logger::log(LOG_DEBUG, "Natives::sql_num_rows: Retrieving the count of rows (stmt->id = %d)...", params[1]);
	return stmt->resultSets[stmt->lastResultIdx]->numRows;
}

cell AMX_NATIVE_CALL Natives::sql_num_fields(AMX *amx, cell *params) {
	if (params[0] < 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	Logger::log(LOG_DEBUG, "Natives::sql_num_fields: Retrieving the count of fields (stmt->id = %d)...", params[1]);
	return stmt->resultSets[stmt->lastResultIdx]->numFields;
}

cell AMX_NATIVE_CALL Natives::sql_next_result(AMX *amx, cell* params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(stmt->connectionId)) {
		return 0;
	}
	Logger::log(LOG_DEBUG, "Natives::sql_next_result: Retrieving next result (stmt->id = %d, next_result = %d)...", params[1], params[2]);
	return SQL_Pools::connections[stmt->connectionId]->seekResult(stmt, params[2]);
}

cell AMX_NATIVE_CALL Natives::sql_field_name(AMX *amx, cell *params) {
	if (params[0] < 4 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(stmt->connectionId)) {
		return 0;
	}
	char *tmp = NULL;
	int len;
	bool isCopy = SQL_Pools::connections[stmt->connectionId]->fetchField(stmt, params[2], tmp, len);
	if (len != 0) {
		if (params[4] < 2) {
			Logger::log(LOG_DEBUG, "Natives::sql_field_name: Specified destination size is smaller than 2, setting it to %d.", len);
			amx_SetCString(amx, params[3], tmp, len);
		} else {
			amx_SetCString(amx, params[3], tmp, params[4]);
		}
		if (isCopy) {
			free(tmp);
		}
	} else {
		Logger::log(LOG_WARNING, "Natives::sql_field_name: Can't find field %d or result is empty.", params[2]);
	}
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_fetch_row(AMX *amx, cell *params) {
	if (params[0] < 4 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(stmt->connectionId)) {
		return 0;
	}
	SQL_Connection *conn = SQL_Pools::connections[stmt->connectionId];
	Logger::log(LOG_DEBUG, "Natives::sql_fetch_row: Fetching a row (stmt->id = %d)...", params[1]);
	char *sep;
	amx_StrParam(amx, params[2], sep);
	char *ret = (char*) malloc(4096 * sizeof(char));
	memset(ret, 0, 4096 * sizeof(char));
	for (int i = 0, fields = stmt->resultSets[stmt->lastResultIdx]->numFields; i != fields; ++i) {
		char *tmp = NULL;
		int len;
		bool isCopy = conn->fetchNum(stmt, i, tmp, len);
		sprintf(ret, "%s%s%s", ret, tmp, sep);
		if (isCopy) {
			free(tmp);
		}
	}
	int len = strlen(ret);
	if (len != 0) {
		if (params[4] < 2) { // Probably a multi-dimensional array.
			Logger::log(LOG_DEBUG, "Natives::sql_fetch_row: Specified destination size is smaller than 2, setting it to %d.", len);
			amx_SetCString(amx, params[3], ret, len);
		} else {
			amx_SetCString(amx, params[3], ret, params[4]);
		}
		free(ret);
	} else {
		Logger::log(LOG_WARNING, "Natives::sql_fetch_row: This row is empty.");
	}
	return 0;
}

cell AMX_NATIVE_CALL Natives::sql_next_row(AMX *amx, cell *params) {
	if (params[0] < 2 * 4) {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(stmt->connectionId)) {
		return 0;
	}
	Logger::log(LOG_DEBUG, "Natives::sql_next_row: Retrieving next row (stmt->id = %d, next_row = %d)...", params[1], params[2]);
	return SQL_Pools::connections[stmt->connectionId]->seekRow(stmt, params[2]);
}

cell AMX_NATIVE_CALL Natives::sql_get_field(AMX *amx, cell *params) {
	cell fieldidx, row, dest_str, dest_len;
	if (params[0] == 4 * 4) {
		row = -1;
		fieldidx = params[2];
		dest_str = params[3];
		dest_len = params[4];
	} else if (params[0] == 5 * 4) {
		row = params[2];
		fieldidx = params[3];
		dest_str = params[4];
		dest_len = params[5];
	} else {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(stmt->connectionId)) {
		return 0;
	}
	SQL_Connection *conn = SQL_Pools::connections[stmt->connectionId];
	if (row != -1) {
		conn->seekRow(stmt, row);
	}
	char *tmp = NULL;
	int len;
	bool isCopy = conn->fetchNum(stmt, fieldidx, tmp, len);
	if (len != 0) {
		if (dest_len < 2) { // Probably a multi-dimensional array.
			Logger::log(LOG_DEBUG, "Natives::sql_get_field: Specified destination size is smaller than 2, setting it to %d.", len);
			amx_SetCString(amx, dest_str, tmp, len);
		} else {
			amx_SetCString(amx, dest_str, tmp, dest_len);
		}
		if (isCopy) {
			free(tmp);
		}
	} else {
		Logger::log(LOG_WARNING, "Natives::sql_get_field: Can't find field %d or result is empty.", fieldidx);
	}
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_assoc(AMX *amx, cell *params) {
	cell fieldidx, row, dest_str, dest_len;
	if (params[0] == 4 * 4) {
		row = -1;
		fieldidx = params[2];
		dest_str = params[3];
		dest_len = params[4];
	} else if (params[0] == 5 * 4) {
		row = params[2];
		fieldidx = params[3];
		dest_str = params[4];
		dest_len = params[5];
	} else {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(stmt->connectionId)) {
		return 0;
	}
	SQL_Connection *conn = SQL_Pools::connections[stmt->connectionId];
	if (row != -1) {
		conn->seekRow(stmt, row);
	}
	char *fieldname = NULL;
	amx_StrParam(amx, fieldidx, fieldname);
	if (fieldname == NULL) {
		Logger::log(LOG_WARNING, "Natives::sql_get_field_assoc: Field name is empty.");
		amx_SetCString(amx, dest_str, "", dest_len);
		return 0;
	}
	char *tmp = NULL;
	int len;
	bool isCopy = conn->fetchAssoc(stmt, fieldname, tmp, len);
	if (len != 0) {
		if (dest_len < 2) {
			Logger::log(LOG_DEBUG, "Natives::sql_get_field_assoc: Specified destination size is smaller than 2, setting it to %d.", len);
			amx_SetCString(amx, dest_str, tmp, len);
		} else {
			amx_SetCString(amx, dest_str, tmp, dest_len);
		}
		if (isCopy) {
			free(tmp);
		}
	} else {
		Logger::log(LOG_WARNING, "Natives::sql_get_field_assoc: Can't find field %s or result is empty.", fieldname);
	}
	return len;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_int(AMX *amx, cell *params) {
	cell fieldidx, row;
	if (params[0] == 2 * 4) {
		row = -1;
		fieldidx = params[2];
	} else if (params[0] == 3 * 4) {
		row = params[2];
		fieldidx = params[3];
	} else {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(stmt->connectionId)) {
		return 0;
	}
	SQL_Connection *conn = SQL_Pools::connections[stmt->connectionId];
	if (row != -1) {
		conn->seekRow(stmt, row);
	}
	char *tmp = NULL;
	int len, val = 0;
	bool isCopy = conn->fetchNum(stmt, fieldidx, tmp, len);
	if (len != 0) {
		val = atoi(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		Logger::log(LOG_WARNING, "Natives::sql_get_field_int: Can't find field %d or result is empty.", fieldidx);
	}
	return val;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_assoc_int(AMX *amx, cell *params) {
	cell fieldidx, row;
	if (params[0] == 2 * 4) {
		row = -1;
		fieldidx = params[2];
	} else if (params[0] == 3 * 4) {
		row = params[2];
		fieldidx = params[3];
	} else {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(stmt->connectionId)) {
		return 0;
	}
	SQL_Connection *conn = SQL_Pools::connections[stmt->connectionId];
	if (row != -1) {
		conn->seekRow(stmt, row);
	}
	char *fieldname = NULL;
	amx_StrParam(amx, fieldidx, fieldname);
	if (fieldname == NULL) {
		Logger::log(LOG_WARNING, "Natives::sql_get_field_assoc: Field name is empty.");
		return 0;
	}
	char *tmp = NULL;
	int len, val = 0;
	bool isCopy = conn->fetchAssoc(stmt, fieldname, tmp, len);
	if (len != 0) {
		val = atoi(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		Logger::log(LOG_WARNING, "Natives::sql_get_field_assoc_int: Can't find field %s or result is empty.", fieldname);
	}
	return val;
}

cell AMX_NATIVE_CALL Natives::sql_get_field_float(AMX *amx, cell *params) {
	cell fieldidx, row;
	if (params[0] == 2 * 4) {
		row = -1;
		fieldidx = params[2];
	} else if (params[0] == 3 * 4) {
		row = params[2];
		fieldidx = params[3];
	} else {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(stmt->connectionId)) {
		return 0;
	}
	SQL_Connection *conn = SQL_Pools::connections[stmt->connectionId];
	if (row != -1) {
		conn->seekRow(stmt, row);
	}
	int len;
	char *tmp = NULL;
	bool isCopy = conn->fetchNum(stmt, fieldidx, tmp, len);
	float val = 0.0;
	if (len != 0) {
		val = (float) atof(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		Logger::log(LOG_WARNING, "Natives::sql_get_field_int: Can't find field %d or result is empty.", fieldidx);
	}
	return amx_ftoc(val);
}

cell AMX_NATIVE_CALL Natives::sql_get_field_assoc_float(AMX *amx, cell *params) {
	cell fieldidx, row;
	if (params[0] == 2 * 4) {
		row = -1;
		fieldidx = params[2];
	} else if (params[0] == 3 * 4) {
		row = params[2];
		fieldidx = params[3];
	} else {
		return 0;
	}
	if (!SQL_Pools::isValidStatement(params[1])) {
		return 0;
	}
	SQL_Statement *stmt = SQL_Pools::statements[params[1]];
	if (stmt->status == STATEMENT_STATUS_NONE) {
		return 0;
	}
	if (!SQL_Pools::isValidConnection(stmt->connectionId)) {
		return 0;
	}
	SQL_Connection *conn = SQL_Pools::connections[stmt->connectionId];
	if (row != -1) {
		conn->seekRow(stmt, row);
	}
	char *fieldname = NULL;
	amx_StrParam(amx, fieldidx, fieldname);
	if (fieldname == NULL) {
		Logger::log(LOG_WARNING, "Natives::sql_get_field_assoc: Field name is empty.");
		return 0;
	}
	char *tmp = NULL;
	int len;
	bool isCopy = conn->fetchAssoc(stmt, fieldname, tmp, len);
	float val = 0.0;
	if (len != 0) {
		val = (float) atof(tmp);
		if (isCopy) {
			free(tmp);
		}
	} else {
		Logger::log(LOG_WARNING, "Natives::sql_get_field_assoc_int: Can't find field %s or result is empty.", fieldname);
	}
	return amx_ftoc(val);
}
