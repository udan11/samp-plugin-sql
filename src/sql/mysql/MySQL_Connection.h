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

#include "mysql.h"
 
#ifdef PLUGIN_SUPPORTS_MYSQL

	#include "../SQL_Connection.h"

	class MySQL_Connection : public SQL_Connection {

		public:
			MySQL_Connection(int id, AMX *amx);
			~MySQL_Connection();
			void startWorker();
			void stopWorker();
			bool connect(const char *host, const char *user, const char *pass, const char *db, int port);
			void disconnect();
			int getErrorId();
			const char *getError();
			int ping();
			const char *getStat();
			const char *getCharset();
			bool setCharset(char *charset);
			int escapeString(const char *src, char *&dest);
			void executeStatement(SQL_Statement *stmt);
			bool seekResult(SQL_Statement *stmt, int resultIdx);
			bool fetchField(SQL_Statement *stmt, int fieldIdx, char *&dest, int &len);
			bool seekRow(SQL_Statement *stmt, int rowIdx);
			bool fetchNum(SQL_Statement *stmt, int fieldIdx, char *&dest, int &len);
			bool fetchAssoc(SQL_Statement *stmt, char *fieldName, char *&dest, int &len);
			
		private:

			/**
			 * MySQL C connector is not (entirely) thread-safe.
			 */
			Mutex *mutex;

			/**
			 * The MySQL connection resource.
			 */
			MYSQL *conn;
	};

#endif
