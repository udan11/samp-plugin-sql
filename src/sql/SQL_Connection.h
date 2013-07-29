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

#pragma once

#include "sql.h"

#ifdef _WIN32
	#include <Windows.h>
	#define SLEEP(x) Sleep(x);
#else
	#include "pthread.h"
	#include <unistd.h>
	#define SLEEP(x) usleep(x * 1000);
	typedef unsigned long DWORD;
	typedef unsigned int UINT;
#endif

#ifdef _WIN32
	DWORD WINAPI SQL_Worker(LPVOID param);
#else
	void *SQL_Worker(void *param);
#endif

/**
 * An abstract SQL connection.
 */
class SQL_Connection {

	public:
	
		/**
		 * SQL's connection unique ID.
		 */
		int id;
	
		/**
		 * The AMX machine owning this connection.
		 */
		AMX *amx;
		
		/**
		 * SQL's connection type (MySQL, PostgreSQL, etc.).
		 */
		int type;
		
		/**
		 * `true` if SQL's worker thread is active, `false` otherwise.
		 */
		bool isActive;
		
		/**
		 * The queue of pending queries, scheduled for execution.
		 */
		statementsQueue_t pending;
		
	#ifdef _WIN32
	
		/**
		 * Win32 thread.
		 */
		HANDLE thread;
	#else
	
		/**
		 * UNIX thread.
		 */
		pthread_t thread;
	#endif
	
		/**
		 * Constructor.
		 * @param id
		 */
		SQL_Connection(int id, AMX *amx);
		
		/**
		 * Destructor.
		 */
		virtual ~SQL_Connection();
		
		/**
		 * Starts the worker thread of this SQL connection.
		 */
		void startWorker();
		
		/**
		 * Stops the worker thread of this SQL connection.
		 */
		void stopWorker();
		
		/**
		 * Establishes a new connection to a SQL server.
		 * @param host
		 * @param user
		 * @param pass
		 * @param db
		 * @param port
		 * @return
		 */
		virtual bool connect(const char *host, const char *user, const char *pass, const char *db, int port) = 0;
		
		/**
		 * Destroys the connection to the SQL server.
		 */
		virtual void disconnect() = 0;
		
		/**
		 * Gets latest error ID returned by the server.
		 * @return
		 */
		virtual int getErrorId() = 0;
		
		/**
		 * Gets latest error message returned by the server.
		 * @return
		 */
		virtual const char *getError() = 0;
		
		/**
		 * Pings the server and returns its status.
		 * @return
		 */
		virtual int ping() = 0;
		
		/**
		 * Gets server's statistics.
		 * @return
		 */
		virtual const char *getStat() = 0;
		
		/**
		 * Gets server's charset.
		 * @return
		 */
		virtual const char *getCharset() = 0;
		
		/**
		 * Sets server's charset.
		 * @param charset
		 * @return
		 */
		virtual bool setCharset(char *charset) = 0;
		
		/**
		 * Escapes a string, making it suitable for SQL queries.
		 * @return
		 */
		virtual int escapeString(const char *src, char *&dest) = 0;
		
		/**
		 * Executes a SQL statement.
		 * @param stmt
		 */
		virtual void executeStatement(SQL_Statement *stmt) = 0;
		
		/**
		 * Seeks a result set.
		 * @param stmt
		 * @param resultIdx
		 * @return
		 */
		virtual bool seekResult(SQL_Statement *stmt, int resultIdx) = 0;
		
		/**
		 * Fetches the name of a field.
		 * @param stmt
		 * @param fieldIdx
		 * @param dest
		 * @param len
		 * @return
		 */
		virtual bool fetchField(SQL_Statement *stmt, int fieldIdx, char *&dest, int &len) = 0;
		
		/**
		 * Seeks a row.
		 * @param stmt
		 * @param rowIdx
		 * @return
		 */
		virtual bool seekRow(SQL_Statement *stmt, int rowIdx) = 0;
		
		/**
		 * Fetches a field by it's index.
		 * @param stmt
		 * @param fieldIdx
		 * @param dest
		 * @param len
		 * @return
		 */
		virtual bool fetchNum(SQL_Statement *stmt, int fieldIdx, char *&dest, int &len) = 0;
		
		/**
		 * Fetches a field by it's name.
		 * @param stmt
		 * @param fieldName
		 * @param dest
		 * @param len
		 * @return
		 */
		virtual bool fetchAssoc(SQL_Statement *stmt, char *fieldName, char *&dest, int &len) = 0;
};
