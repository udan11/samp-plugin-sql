#pragma once

#include "main.h"

class MySQL_Handler {
	public:
		// The AMX machine hosting this handler.
		AMX *amx;
		// The ID of this handler.
		int id;
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
		const char* get_error();
		// Pings the servers.
		int ping();
		// Gets server's information.
		const char *get_info();
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
		int fetch_field(struct mysql_query *query, int fieldix, char *&dest);
		// Seeks a row in the result.
		int seek_row(struct mysql_query *query, int row);
		// Fetches a cell by it's index.
		int fetch_num(struct mysql_query *query, int fieldidx, char *&dest);
		// Fetches a cell by it's name.
		int fetch_assoc(struct mysql_query *query, char *fieldname, char *&dest);
	private:
		// The MySQL client socket.
		MYSQL *conn;
};