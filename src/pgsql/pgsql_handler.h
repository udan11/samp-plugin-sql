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

#pragma once

class PgSQL_Handler : public SQL_Handler {
	public:
		PgSQL_Handler();
		~PgSQL_Handler();
		bool connect(const char *host, const char *user, const char *pass, const char *db, int port);
		void disconnect();
		int get_errno();
		const char *get_error();
		int ping();
		const char *get_stat();
		const char *get_charset();
		bool set_charset(char *charset);
		int escape_string(const char *src, char *&dest);
		void execute_query(SQL_Query *query);
		bool seek_result(SQL_Query *query, int result);
		bool fetch_field(SQL_Query *query, int fieldix, char *&dest, int &len);
		bool seek_row(SQL_Query *query, int row);
		bool fetch_num(SQL_Query *query, int fieldidx, char *&dest, int &len);
		bool fetch_assoc(SQL_Query *query, char *fieldname, char *&dest, int &len);
	private:
		PGconn *conn;
};

#endif