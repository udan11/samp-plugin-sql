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
	// TODO
}

PgSQL_Handler::~PgSQL_Handler() {
	// TODO
}

bool PgSQL_Handler::connect(const char *host, const char *user, const char *pass, const char *db, int port = 3306) {
	// TODO
	return false;
}

void PgSQL_Handler::disconnect() {
	// TODO
}

int PgSQL_Handler::get_errno() {
	// TODO
	return 0;
}

const char *PgSQL_Handler::get_error() {
	// TODO
	return 0;
}

int PgSQL_Handler::ping() {
	// TODO
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
	// TODO
	return 0;
}

void PgSQL_Handler::execute_query(class SQL_Query *query) {
	// TODO
}

bool PgSQL_Handler::fetch_field(class SQL_Query *query, int fieldidx, char *&dest, int &len) {
	// TODO
	return false;
}

bool PgSQL_Handler::seek_row(class SQL_Query *query, int row) {
	// TODO
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