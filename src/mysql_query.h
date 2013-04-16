/**
 * SA:MP Plugin - MySQL
 * Copyright (C) 2013 Dan
 *  
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "main.h"

#define QUERY_THREADED						1
#define QUERY_CACHED						2

#define QUERY_STATUS_NONE					0
#define QUERY_STATUS_EXECUTED				1
#define QUERY_STATUS_PROCESSED				2

struct mysql_query {
	int id, handler, flags, status, error;
	char *query, *callback, *format;
	const char *error_msg;
	std::vector<cell> params_c;
	std::vector<char*> params_s;
	MYSQL_RES *result;
	MYSQL_ROW last_row;
	unsigned long *last_row_lengths;
	int last_row_idx, insert_id, affected_rows, num_rows, num_fields;
	std::vector<std::pair<char*, int > > field_names;
	std::vector<std::vector<std::pair<char*, int> > > cache;
};

extern void free_query(struct mysql_query *&query);
extern int execute_query_callback(struct mysql_query *query);