/**
 * MySQL plugin v0.2 BETA
 */
 
/**
 * Tags:
 *
 * MySQL		a MySQL connection.
 * Result		a MySQL result.
 */

// Possible values of mysql_ping().
#define CR_UNKNOWN_ERROR				2000
#define CR_SERVER_GONE_ERROR			2006
#define CR_COMMANDS_OUT_OF_SYNC			2014

// "Compatibilty" with SQLite
#define mysql_open						mysql_connect
#define mysql_close						mysql_disconnect

// Query flags.
enum {
	QUERY_NONE							= 0b00, // No flag = non-threaded & non-cached.
	QUERY_THREADED						= 0b01,
	QUERY_CACHED						= 0b10
};

// Callbacks.
forward OnMySQLError(MySQL:handle, errorid, const error[], const query[], const callback[]);

// Connect and disconnect from the MySQL server.
native MySQL:mysql_connect(const host[], const user[], const pass[], const db[], port = 3306);
native mysql_disconnect(MySQL:handle);

// Pings the MySQL server.
native mysql_ping(MySQL:handle);

// Escapes a MySQL string.
native mysql_escape_string(MySQL:handle, const src[], dest[], dest_len = sizeof(dest));

// Executes a MySQL query and returns the result.
// Supported formats: 
//		b, B = bool
// 		c, C = character
//		d, D, i, I = integer
// 		s, S = string
// 		r, R = result
native Result:mysql_query(MySQL:handle, const query[], flag = QUERY_NONE, const callback[] = "", const format[] = "", {Float,_}:...);

// Stores the result for later use (if query is threaded).
native mysql_store_result(Result:result);
// Frees the result for later use (if the query was stored or is not threaded).
native mysql_free_result(Result:result);

// Gets the ID of the last insert query.
native mysql_insert_id(Result:result);

// Gets the ID / message of the error.
native mysql_error(Result:result);
native mysql_error_string(Result:result);

// Counts the rows and fields.
native mysql_num_rows(Result:result);
native mysql_num_fields(Result:result);

// Fetches the name of a field.
native mysql_field_name(Result:result, field, dest[], dest_len = sizeof(dest));

// Fetches next row.
native mysql_next_row(Result:result, row = -1);

// Fetches a string cell.
native mysql_get_field(Result:result, field, dest[], dest_len = sizeof(dest));
native mysql_get_field_assoc(Result:result, const field[], dest[], dest_len = sizeof(dest));

// Fetches an integer cell.
native mysql_get_field_int(Result:result, field);
native mysql_get_field_assoc_int(Result:result, const field[]);

// Fetches a float cell.
native Float:mysql_get_field_float(Result:result, field);
native Float:mysql_get_field_assoc_float(Result:result, const field[]);