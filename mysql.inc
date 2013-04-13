/**
 * MySQL plugin v0.4 BETA
 *
 * <remarks>
 * 		Data types:
 * 			MySQL 	- a handle to one database connection
 * 			Result 	- the result of a query, whether it returns or not rows
 * </remarks>
 */

/**
 * <summary>Client error codes.</summary>
 */
#define CR_UNKNOWN_ERROR						 	2000
#define CR_SOCKET_CREATE_ERROR					 	2001
#define CR_CONNECTION_ERROR						 	2002
#define CR_CONN_HOST_ERROR						 	2003
#define CR_IPSOCK_ERROR							 	2004
#define CR_UNKNOWN_HOST							 	2005
#define CR_SERVER_GONE_ERROR					 	2006
#define CR_VERSION_ERROR						 	2007
#define CR_OUT_OF_MEMORY						 	2008
#define CR_WRONG_HOST_INFO						 	2009
#define CR_LOCALHOST_CONNECTION					 	2010
#define CR_TCP_CONNECTION						 	2011
#define CR_SERVER_HANDSHAKE_ERR					 	2012
#define CR_SERVER_LOST							 	2013
#define CR_COMMANDS_OUT_OF_SYNC					 	2014
#define CR_NAMEDPIPE_CONNECTION					 	2015
#define CR_NAMEDPIPEWAIT_ERROR					 	2016
#define CR_NAMEDPIPEOPEN_ERROR					 	2017
#define CR_NAMEDPIPESETSTATE_ERROR				 	2018
#define CR_CANT_READ_CHARSET					 	2019
#define CR_NET_PACKET_TOO_LARGE					 	2020
#define CR_EMBEDDED_CONNECTION					 	2021
#define CR_PROBE_SLAVE_STATUS					 	2022
#define CR_PROBE_SLAVE_HOSTS					 	2023
#define CR_PROBE_SLAVE_CONNECT					 	2024
#define CR_PROBE_MASTER_CONNECT					 	2025
#define CR_SSL_CONNECTION_ERROR					 	2026
#define CR_MALFORMED_PACKET						 	2027
#define CR_WRONG_LICENSE						 	2028
#define CR_NULL_POINTER							 	2029
#define CR_NO_PREPARE_STMT						 	2030
#define CR_PARAMS_NOT_BOUND						 	2031
#define CR_DATA_TRUNCATED						 	2032
#define CR_NO_PARAMETERS_EXISTS					 	2033
#define CR_INVALID_PARAMETER_NO					 	2034
#define CR_INVALID_BUFFER_USE					 	2035
#define CR_UNSUPPORTED_PARAM_TYPE				 	2036
#define CR_SHARED_MEMORY_CONNECTION				 	2037
#define CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR	 	2038
#define CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR	 	2039
#define CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR	 	2040
#define CR_SHARED_MEMORY_CONNECT_MAP_ERROR		 	2041
#define CR_SHARED_MEMORY_FILE_MAP_ERROR			 	2042
#define CR_SHARED_MEMORY_MAP_ERROR				 	2043
#define CR_SHARED_MEMORY_EVENT_ERROR			 	2044
#define CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR 	2045
#define CR_SHARED_MEMORY_CONNECT_SET_ERROR		 	2046
#define CR_CONN_UNKNOW_PROTOCOL					 	2047
#define CR_INVALID_CONN_HANDLE					 	2048
#define CR_SECURE_AUTH							 	2049
#define CR_FETCH_CANCELED						 	2050
#define CR_NO_DATA								 	2051
#define CR_NO_STMT_METADATA						 	2052

/**
 * <summary>The query will be executed in server's thread and the result is fetched on demand.</summary>
 */
#define QUERY_NONE						0

/**
 * <summary>Runs the query in a different thread (other than main server's thread).</summary>
 */
#define QUERY_THREADED					1

/**
 * <summary>Stores (temporarily) rows returned by a query in cache.</summary>
 */
#define QUERY_CACHED					2

/**
 * <summary>Log levels. (@see mysql_debug)</summary>
 */
#define LOG_ALL							0
#define LOG_DEBUG						1
#define LOG_INFO						2
#define LOG_WARNING						3
#define LOG_ERROR						4
#define LOG_NONE						5

/**
 * <summary>Checks if a string is null.</summary>
 */
#if !defined isnull
	#define isnull(%1) \
		((!(%1[0])) || (((%1[0]) == '\1') && (!(%1[1]))))
#endif

/**
 * <summary>Checks if a cell from a MySQL table is null.</summary>
 */
#if !defined ismysqlnull
	#define ismysqlnull(%1) \
		((isnull(%1)) || (strcmp(%1, "NULL", false) == 0))
#endif
	
/**
 * <summary>Aliases of natives defined below.</summary>
 */
#define mysql_open						mysql_connect
#define mysql_close						mysql_disconnect
#define mysql_fetch_num					mysql_get_field
#define mysql_fetch_assoc				mysql_get_field_assoc
#define mysql_fetch_num_int				mysql_get_field_int
#define mysql_fetch_assoc_int			mysql_get_field_assoc_int
#define mysql_fetch_num_float			mysql_get_field_int_float
#define mysql_fetch_assoc_float			mysql_get_field_assoc_float

/**
 * <summary>Called when a MySQL error occurs during the execution of a query.</summary>
 * <param name="handle">The MySQL handle used for execution of the query.</param>
 * <param name="errorid">The ID of the error (@see "C.4. Client Error Codes and Messages" from "MySQL Reference Manual").</param>
 * <param name="error">The message of the error.</param>
 * <param name="query">Query which caused the error.</param>
 * <param name="callback">Callback which should have been called normally.</param>
 * <returns>No return value is expected.</returns>
 */
forward OnMySQLError(MySQL:handle, errorid, const error[], const query[], const callback[]);

/**
 * <summary>Sets the minimum level of logged messages.</summary>
 * <param name="file">The minimum level of the file (mysql_log.txt) logs.</param>
 * <param name="console">The minimum level of the console logs.</param>
 * <returns>No return value.</returns>
 */
native mysql_debug(file = LOG_ALL, console = LOG_WARNING);

/**
 * <summary>Creates a new handle and connects to a MySQL server.</summary>
 * <param name="host">The MySQL hostname.</param>
 * <param name="user">The MySQL username.</param>
 * <param name="pass">The MySQL password assigned to the user used.</param>
 * <param name="db">The name of the targeted database.</param>
 * <param name="port">The port on which the MySQL server listens.</param>
 * <returns>The ID of the handle.</returns>
 */
native MySQL:mysql_connect(const host[], const user[], const pass[], const db[], port = 3306);

/**
 * <summary>Destroys the handle and disconnects from the MySQL server.</summary>
 * <param name="handle">The MySQL handle which has to be disconnected.</param>
 * <returns>No return value.</returns>
 */
native mysql_disconnect(MySQL:handle);

/**
 * <summary>Sets default character set.</sumarry>
 * <param name="handle">The MySQL handle.</param>
 * <param name="charset">New default character set.</param>
 * <returns>True if succesful.</returns>
 */
native mysql_set_charset(MySQL:handle, const charset[]);

/**
 * <summary>Gets default character set.</sumarry>
 * <param name="dest">The destination where the character set will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>True if succesful.</returns>
 */
native mysql_get_charset(MySQL:handle, dest[], dest_len = sizeof(dest));

/**
 * <summary>Pings the MySQL server.</summary>
 * <param name="handle">The MySQL handle which has to be disconnected.</param>
 * <returns>Returns a MySQL client error (if any) or 0 if the connection is alive.</returns>
 */
native mysql_ping(MySQL:handle);

/**
 * <summary>Gets statistics from MySQL server.</sumarry>
 * <param name="dest">The destination where the information will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>True if succesful.</returns>
 */
native mysql_get_stat(MySQL:handle, dest[], dest_len = sizeof(dest));

/**
 * <summary>Pings the MySQL server.</summary>
 * <param name="handle">The MySQL handle used for escaping the string.</param>
 * <param name="src">The source of the unescaped string.</param>
 * <param name="dest">The destination where the escaped string will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The length of the escaped string.</returns>
 */
native mysql_escape_string(MySQL:handle, const src[], dest[], dest_len = sizeof(dest));

/**
 * <summary>Executes a MySQL query and returns the result.</summary>
 * <param name="handle">The MySQL handle used for execution of the query.</param>
 * <param name="query">The query.</param>
 * <param name="flag">Query's flags.</param>
 * <param name="callback">The callback which has to be called after the query was sucesfully executed.</param>
 * <param name="format">The format of the callback (b, B = boolean; c, C = character; d, D, i, I = integer; s, S = string; r, R = result).</param>
 * <returns>The ID of the result.</returns>
 */
native Result:mysql_query(MySQL:handle, const query[], flag = QUERY_NONE, const callback[] = "", const format[] = "", {Float,_}:...);

/**
 * <summary>Stores the result for later use (if query is threaded).</summary>
 * <param name="result">The ID of the result which has to be stored.</param>
 * <returns>No return value.</returns>
 */
native mysql_store_result(Result:result);

/**
 * <summary>Frees the result for later use (if the query was stored or is not threaded).</summary>
 * <param name="result">The ID of the result which has to be freed.</param>
 * <returns>No return value.</returns>
 */
native mysql_free_result(Result:result);

/**
 * <summary>Gets the ID of the last insert query.</summary>
 * <param name="result">The ID of the result.</param>
 * <returns>The ID of the latest inserted row.</returns>
 */
native mysql_insert_id(Result:result);

/**
 * <summary>Gets the ID of the error returned by this result.</summary>
 * <param name="result">The ID of the result.</param>
 * <returns>The ID of the error.</returns>
 */
native mysql_error(Result:result);

/**
 * <summary>Gets the message of the error returned by this result.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="dest">The destination where the error message will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The ID of the error.</returns>
 */
native mysql_error_string(Result:result, dest[], dest_len = sizeof(dest));

/**
 * <summary>Gets the count of rows contained in a result.</summary>
 * <param name="result">The ID of the result.</param>
 * <returns>The count of rows contained in the result.</returns>
 */
native mysql_num_rows(Result:result);

/**
 * <summary>Gets the count of fields contained in a result.</summary>
 * <param name="result">The ID of the result.</param>
 * <returns>The count of fields contained in the result.</returns>
 */
native mysql_num_fields(Result:result);

/**
 * <summary>Fetches the name of a field.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The index of the field.</param>
 * <param name="dest">The destination where the field will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The count of fields contained in the result.</returns>
 */
native mysql_field_name(Result:result, field, dest[], dest_len = sizeof(dest));

/**
 * <summary>Seeks a specific row.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="row">The index of the row. If -1 is specified, next row will be seeked.</param>
 * <returns>No return value.</returns>
 */
native mysql_next_row(Result:result, row = -1);

/**
 * <summary>Fetches a cell containing a string by field index.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The index of the field.</param>
 * <param name="dest">The destination where the string will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The length of the string.</returns>
 */
native mysql_get_field(Result:result, field, dest[], dest_len = sizeof(dest));

/**
 * <summary>Fetches a cell containing a string by field name.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The name of the field.</param>
 * <param name="dest">The destination where the string will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The length of the string.</returns>
 */
native mysql_get_field_assoc(Result:result, const field[], dest[], dest_len = sizeof(dest));

/**
 * <summary>Fetches a cell containing an integer value by field index.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The index of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native mysql_get_field_int(Result:result, field);

/**
 * <summary>Fetches a cell containing an integer value by field name.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The name of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native mysql_get_field_assoc_int(Result:result, const field[]);

/**
 * <summary>Fetches a cell containing a float value by field index.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The index of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native Float:mysql_get_field_float(Result:result, field);

/**
 * <summary>Fetches a cell containing a float value by field name.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The name of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native Float:mysql_get_field_assoc_float(Result:result, const field[]);