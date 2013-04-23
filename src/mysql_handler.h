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

class MySQL_Handler {
	public:
		// The AMX machine hosting this handler.
		AMX *amx;
		// Constructor.
		MySQL_Handler();
		// Destructor.
		~MySQL_Handler();
		// Connects to a MySQL server and returns handler's ID.
		bool connect(const char *host, const char *user, const char *pass, const char *db, int port);
		// Disconnects from the MySQL server.
		void disconnect();
		// Gets latest error ID.
		int get_errno();
		// Gets latest error message.
		const char *get_error();
		// Pings the servers.
		int ping();
		// Gets server's information.
		const char *get_stat();
		// Gets default character set.
		const char *get_charset();
		// Sets default character set.
		bool set_charset(char *charset);
		// Escapes a string and returns the new length.
		int escape_string(const char *src, char *&dest);
		// Executes a query.
		void execute_query(struct mysql_query *&query);
		// Gets the ID of the row inserted.
		int get_insert_id(struct mysql_query *query);
		// Counts the affected rows.
		int get_affected_rows(struct mysql_query *query);
		// Counts the rows returned.
		int get_num_rows(struct mysql_query *query);
		// Counts the rows returned.
		int get_num_fields(struct mysql_query *query);
		// Fetches the name of a field.
		bool fetch_field(struct mysql_query *query, int fieldix, char *&dest, int &len);
		// Seeks a row in the result.
		bool seek_row(struct mysql_query *query, int row);
		// Fetches a cell by it's index.
		bool fetch_num(struct mysql_query *query, int fieldidx, char *&dest, int &len);
		// Fetches a cell by it's name.
		bool fetch_assoc(struct mysql_query *query, char *fieldname, char *&dest, int &len);
	private:
		// The MySQL client socket.
		MYSQL *conn;
};