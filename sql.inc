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
 
/**
 * <version>2.6</version>
 * <remarks>
 * Data types:
 * 		SQL	- a SQL database handle
 * 		Result	- the result of a query, whether it returns or not rows
 * </remarks>
 */

/**
 * <summary>MySQL client error codes.</summary>
 */
#define MYSQL_CR_UNKNOWN_ERROR								2000
#define MYSQL_CR_SOCKET_CREATE_ERROR						2001
#define MYSQL_CR_CONNECTION_ERROR							2002
#define MYSQL_CR_CONN_HOST_ERROR							2003
#define MYSQL_CR_IPSOCK_ERROR								2004
#define MYSQL_CR_UNKNOWN_HOST								2005
#define MYSQL_CR_SERVER_GONE_ERROR							2006
#define MYSQL_CR_VERSION_ERROR								2007
#define MYSQL_CR_OUT_OF_MEMORY								2008
#define MYSQL_CR_WRONG_HOST_INFO							2009
#define MYSQL_CR_LOCALHOST_CONNECTION						2010
#define MYSQL_CR_TCP_CONNECTION								2011
#define MYSQL_CR_SERVER_HANDSHAKE_ERR						2012
#define MYSQL_CR_SERVER_LOST								2013
#define MYSQL_CR_COMMANDS_OUT_OF_SYNC						2014
#define MYSQL_CR_NAMEDPIPE_CONNECTION						2015
#define MYSQL_CR_NAMEDPIPEWAIT_ERROR						2016
#define MYSQL_CR_NAMEDPIPEOPEN_ERROR						2017
#define MYSQL_CR_NAMEDPIPESETSTATE_ERROR					2018
#define MYSQL_CR_CANT_READ_CHARSET							2019
#define MYSQL_CR_NET_PACKET_TOO_LARGE						2020
#define MYSQL_CR_EMBEDDED_CONNECTION						2021
#define MYSQL_CR_PROBE_SLAVE_STATUS							2022
#define MYSQL_CR_PROBE_SLAVE_HOSTS							2023
#define MYSQL_CR_PROBE_SLAVE_CONNECT						2024
#define MYSQL_CR_PROBE_MASTER_CONNECT						2025
#define MYSQL_CR_SSL_CONNECTION_ERROR						2026
#define MYSQL_CR_MALFORMED_PACKET							2027
#define MYSQL_CR_WRONG_LICENSE								2028
#define MYSQL_CR_NULL_POINTER								2029
#define MYSQL_CR_NO_PREPARE_STMT							2030
#define MYSQL_CR_PARAMS_NOT_BOUND							2031
#define MYSQL_CR_DATA_TRUNCATED								2032
#define MYSQL_CR_NO_PARAMETERS_EXISTS						2033
#define MYSQL_CR_INVALID_PARAMETER_NO						2034
#define MYSQL_CR_INVALID_BUFFER_USE							2035
#define MYSQL_CR_UNSUPPORTED_PARAM_TYPE						2036
#define MYSQL_CR_SHARED_MEMORY_CONNECTION					2037
#define MYSQL_CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR		2038
#define MYSQL_CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR			2039
#define MYSQL_CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR		2040
#define MYSQL_CR_SHARED_MEMORY_CONNECT_MAP_ERROR			2041
#define MYSQL_CR_SHARED_MEMORY_FILE_MAP_ERROR				2042
#define MYSQL_CR_SHARED_MEMORY_MAP_ERROR					2043
#define MYSQL_CR_SHARED_MEMORY_EVENT_ERROR					2044
#define MYSQL_CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR 		2045
#define MYSQL_CR_SHARED_MEMORY_CONNECT_SET_ERROR			2046
#define MYSQL_CR_CONN_UNKNOW_PROTOCOL						2047
#define MYSQL_CR_INVALID_CONN_HANDLE						2048
#define MYSQL_CR_SECURE_AUTH								2049
#define MYSQL_CR_FETCH_CANCELED								2050
#define MYSQL_CR_NO_DATA									2051
#define MYSQL_CR_NO_STMT_METADATA							2052

/**
 * <summary>MySQL</summary>
 */
#define SQL_HANDLER_MYSQL				1

/**
 * <summary>Postgre SQL</summary>
 */
#define SQL_HANDLER_POSTGRESQL			2

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
 * <summary>Log levels. (@see sql_debug)</summary>
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
 * <summary>Checks if a cell from a SQL table is null.</summary>
 */
#if !defined issqlnull
	#define issqlnull(%1) \
		((isnull(%1)) || (strcmp(%1, "NULL", false) == 0))
#endif
	
/**
 * <summary>Aliases of natives defined below.</summary>
 */
#define sql_open						sql_connect
#define sql_close						sql_disconnect
#define mysql_connect(%0)				sql_connect(SQL_HANDLER_MYSQL,%0)
#define pgsql_connect(%0)				sql_connect(SQL_HANDLER_POSTGRESQL,%0)

/**
 * <summary>Called when a SQL error occurs during the execution of a query.</summary>
 * <param name="handle">The SQL handle used for execution of the query.</param>
 * <param name="errorid">The ID of the error (@see "C.4. Client Error Codes and Messages" from "SQL Reference Manual").</param>
 * <param name="error">The message of the error.</param>
 * <param name="query">Query which caused the error.</param>
 * <param name="callback">Callback which should have been called normally.</param>
 * <returns>No return value is expected.</returns>
 */
forward OnSQLError(SQL:handle, errorid, error[], query[], callback[]);

/**
 * <summary>Sets the minimum level of logged messages.</summary>
 * <param name="file">The minimum level of the file (sql_log.txt) logs.</param>
 * <param name="console">The minimum level of the console logs.</param>
 * <returns>No return value.</returns>
 */
native sql_debug(file = LOG_ALL, console = LOG_WARNING);

/**
 * <summary>Creates a new handle and connects to a SQL server.</summary>
 * <param name="host">The SQL hostname.</param>
 * <param name="user">The SQL username.</param>
 * <param name="pass">The SQL password assigned to the user used.</param>
 * <param name="db">The name of the targeted database.</param>
 * <param name="port">The port on which the SQL server listens.</param>
 * <returns>The ID of the handle.</returns>
 */
native SQL:sql_connect(sql_type, host[], user[], pass[], db[], port = 0);

/**
 * <summary>Destroys the handle and disconnects from the SQL server.</summary>
 * <param name="handle">The SQL handle which has to be disconnected.</param>
 * <returns>No return value.</returns>
 */
native sql_disconnect(SQL:handle);

/**
 * <summary>Waits for a handle to finish its activity (all queries to be executed).</summary>
 * <param name="handle">The SQL handle.</param>
 * <returns>No return value.</returns>
 */
native sql_wait(SQL:handle);

/**
 * <summary>Sets default character set.</summary>
 * <param name="handle">The SQL handle.</param>
 * <param name="charset">New default character set.</param>
 * <returns>True if succesful.</returns>
 */
native sql_set_charset(SQL:handle, charset[]);

/**
 * <summary>Gets default character set.</summary>
 * <param name="dest">The destination where the character set will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>True if succesful.</returns>
 */
native sql_get_charset(SQL:handle, dest[], dest_len = sizeof(dest));

/**
 * <summary>Pings the SQL server.</summary>
 * <param name="handle">The SQL handle which has to be disconnected.</param>
 * <returns>Returns a SQL client error (if any) or 0 if the connection is alive.</returns>
 */
native sql_ping(SQL:handle);

/**
 * <summary>Gets statistics from SQL server.</summary>
 * <param name="dest">The destination where the information will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>True if succesful.</returns>
 */
native sql_get_stat(SQL:handle, dest[], dest_len = sizeof(dest));

/**
 * <summary>Escapes a string to be used further in queries.</summary>
 * <param name="handle">The SQL handle used for escaping the string.</param>
 * <param name="src">The source of the unescaped string.</param>
 * <param name="dest">The destination where the escaped string will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The length of the escaped string.</returns>
 */
native sql_escape_string(SQL:handle, src[], dest[], dest_len = sizeof(dest));

/**
 * <summary>Executes a SQL query and returns the result.</summary>
 * <param name="handle">The SQL handle used for execution of the query.</param>
 * <param name="query">The query.</param>
 * <param name="flag">Query's flags.</param>
 * <param name="callback">The callback which has to be called after the query was sucesfully executed.</param>
 * <param name="format">The format of the callback.
 * 		a, A = arrays (must be followed by an integer: array's szie);
 * 		b, B = boolean; c, C = character; d, D, i, I = integer;
 * 		r, R = result; s, S = string
 * </param>
 * <returns>The ID of the result.</returns>
 */
native Result:sql_query(SQL:handle, query[], flag = QUERY_NONE, callback[] = "", format[] = "", {Float,_}:...);

/**
 * <summary>Stores the result for later use (if query is threaded).</summary>
 * <param name="result">The ID of the result which has to be stored.</param>
 * <returns>No return value.</returns>
 */
native sql_store_result(Result:result);

/**
 * <summary>Frees the result for later use (if the query was stored or is not threaded).</summary>
 * <param name="result">The ID of the result which has to be freed.</param>
 * <returns>No return value.</returns>
 */
native sql_free_result(Result:result);

/**
 * <summary>Gets the count of affected rows.</summary>
 * <param name="result">The ID of the result.</param>
 * <returns>The count of affected rows.</returns>
 */
native sql_affected_rows(Result:result);

/**
 * <summary>Gets the ID of the last insert query.</summary>
 * <param name="result">The ID of the result.</param>
 * <returns>The ID of the latest inserted row.</returns>
 */
native sql_insert_id(Result:result);

/**
 * <summary>Gets the ID of the error returned by this result.</summary>
 * <param name="result">The ID of the result.</param>
 * <returns>The ID of the error.</returns>
 */
native sql_error(Result:result);

/**
 * <summary>Gets the message of the error returned by this result.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="dest">The destination where the error message will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The ID of the error.</returns>
 */
native sql_error_string(Result:result, dest[], dest_len = sizeof(dest));

/**
 * <summary>Gets the count of rows contained in a result.</summary>
 * <param name="result">The ID of the result.</param>
 * <returns>The count of rows contained in the result.</returns>
 */
native sql_num_rows(Result:result);

/**
 * <summary>Gets the count of fields contained in a result.</summary>
 * <param name="result">The ID of the result.</param>
 * <returns>The count of fields contained in the result.</returns>
 */
native sql_num_fields(Result:result);

/**
 * <summary>Jumps to a specific result set.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="idx">The index of the result set. If -1 is specified, next result set will be retrieved.</param>
 * <returns>`true` if there are any results left, `false` otherwise.</returns>
 */
native sql_next_result(Result:result, idx = -1);

/**
 * <summary>Fetches the name of a field.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The index of the field.</param>
 * <param name="dest">The destination where the field will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The length of field's name.</returns>
 */
native sql_field_name(Result:result, field, dest[], dest_len = sizeof(dest));

/**
 * <summary>Fetches an entire row inserting the separator between each cell.</summary>
 * <param name="result"></param>
 * <param name="separator"></param>
 * <param name="dest">The destination where the field will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
  * <returns>The length of the row.</returns>
 */
native sql_fetch_row(Result:result, sep[], dest[], dest_len = sizeof(dest));

/**
 * <summary>Jumps to a specific row.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="row">The index of the row. If -1 is specified, next row will be retrieved.</param>
 * <returns>`true` if there are any rows left, `false` otherwise.</returns>
 */
native sql_next_row(Result:result, row = -1);

// ----------------------------------------------------------------------------

/**
 * <summary>Fetches a cell containing a string by field index.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The index of the field.</param>
 * <param name="dest">The destination where the string will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The length of the string.</returns>
 */
native sql_get_field(Result:result, field, dest[], dest_len = sizeof(dest));

/**
 * <summary>Fetches a cell containing a string by field name.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The name of the field.</param>
 * <param name="dest">The destination where the string will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The length of the string.</returns>
 */
native sql_get_field_assoc(Result:result, field[], dest[], dest_len = sizeof(dest));

/**
 * <summary>Fetches a cell containing an integer value by field index.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The index of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native sql_get_field_int(Result:result, field);

/**
 * <summary>Fetches a cell containing an integer value by field name.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The name of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native sql_get_field_assoc_int(Result:result, field[]);

/**
 * <summary>Fetches a cell containing a float value by field index.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The index of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native Float:sql_get_field_float(Result:result, field);

/**
 * <summary>Fetches a cell containing a float value by field name.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="field">The name of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native Float:sql_get_field_assoc_float(Result:result, field[]);

// ----------------------------------------------------------------------------

/**
 * <summary>Fetches a cell containing a string by field index.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="row">The index of the row.</param>
 * <param name="field">The index of the field.</param>
 * <param name="dest">The destination where the string will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The length of the string.</returns>
 */
native sql_get_field_ex(Result:result, row, field, dest[], dest_len = sizeof(dest));

/**
 * <summary>Fetches a cell containing a string by field name.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="row">The index of the row.</param>
 * <param name="field">The name of the field.</param>
 * <param name="dest">The destination where the string will be stored.</param>
 * <param name="dest_len">The capacity of the destination.</param>
 * <returns>The length of the string.</returns>
 */
native sql_get_field_assoc_ex(Result:result, row, field[], dest[], dest_len = sizeof(dest));

/**
 * <summary>Fetches a cell containing an integer value by field index.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="row">The index of the row.</param>
 * <param name="field">The index of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native sql_get_field_int_ex(Result:result, row, field);

/**
 * <summary>Fetches a cell containing an integer value by field name.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="row">The index of the row.</param>
 * <param name="field">The name of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native sql_get_field_assoc_int_ex(Result:result, row, field[]);

/**
 * <summary>Fetches a cell containing a float value by field index.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="row">The index of the row.</param>
 * <param name="field">The index of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native Float:sql_get_field_float_ex(Result:result, row, field);

/**
 * <summary>Fetches a cell containing a float value by field name.</summary>
 * <param name="result">The ID of the result.</param>
 * <param name="row">The index of the row.</param>
 * <param name="field">The name of the field.</param>
 * <returns>The value of the cell.</returns>
 */
native Float:sql_get_field_assoc_float_ex(Result:result, row, field[]);

// ----------------------------------------------------------------------------

/**
 * y_inline interface
 */

#if !defined SQL_USE_Y_INLINE
	// NOTE: #emit ignores pre-processor's directives.
	// So, if we disable y_inline support, the #emit instructions below still
	// requirea variable to store the address of the first passed parameter.
	new SQL_addr;
	#pragma unused SQL_addr
#endif

#if defined SQL_USE_Y_INLINE

	#if !defined SQL_MAX_INLINE_CALLBACKS
	
		/**
		 * <summary>The maximum number of inline callbacks scheduled simultaneously.</summary>
		 */
		#define SQL_MAX_INLINE_CALLBACKS	512
	#endif
	
	#if !defined SQL_MAX_INLINE_PARAMETERS
		
		/**
		 * <summary>The maximum number of parameters an inline function can have.</summary>
		 */
		#define SQL_MAX_INLINE_PARAMETERS	16
	#endif

	/**
	 * <summary>Executes a SQL query and calls an inline callback on return.</summary>
	 * <remarks>The "format" parameter must be static.</remarks>
	 */
	//native Result:sql_query_inline(SQL:handle, query[], flag, callback:callback, format[] = "", {Float,_}:...);
	#define sql_query_inline(%0,%1,%2,%3,"%4"%5) \
		sql_query(%0,%1,%2,"SQL_OnInlineCallback","i"#%4,SQL_SaveCallback(%3)%5)

	/**
	 * <summary>Stores information about callbacks that are scheduled to be executed.</summary>
	 * <remarks>This data is recycled. Once `SQL_LoadCallback` was called, that index is marked as free.</remarks>
	 *
	 * TODO: If the query fails, the `SQL_OnInlineCallback` won't  be called
	 * and the data won't be recycled. Fix it.
	 */
	stock SQL_callbacks[SQL_MAX_INLINE_CALLBACKS][E_CALLBACK_DATA];

	/**
	 * <summary>Saves the callback data of a scheduled statement.</summary>
	 * <param name="callback">Callback's ID.</param>
	 * <returns>The index from `SQL_callbacks` where the callback was saved.</returns>
	 */
	stock SQL_SaveCallback(callback:callback) {
		for (new i = 0; i != sizeof(SQL_callbacks); ++i) {
			if (SQL_callbacks[i][E_CALLBACK_DATA_POINTER] != 0) {
				continue;
			}
			if (!Callback_Get(callback, SQL_callbacks[i])) {
				SQL_callbacks[i][E_CALLBACK_DATA_POINTER] = 0;
				return -1;
			}
			return i;
		}
		print("[plugin.sql][warning] No more free space in `SQL_callbacks`.");
		return -1;
	}

	/**
	 * <summary>The Interface between the plugin and y_inline, which loads the callback data and executes it.</summary>
	 * <remarks>This function also recycles data.</remarks>
	 * <param name="idx">Callback's index from `SQL_callbacks`.</param>
	 */
	forward SQL_OnInlineCallback(idx, {Float,_}:...);
	public SQL_OnInlineCallback(idx, {Float,_}:...) {
		if (idx == -1) {
			return;
		}
		new numArgs = numargs(), SQL_addr[SQL_MAX_INLINE_PARAMETERS];
		if (numArgs > 1) {
			#emit CONST.alt			16
			#emit LCTRL				5
			#emit ADD
			#emit STOR.S.pri		SQL_addr
			--numArgs;
			for (new i = 1; i != numArgs; ++i) {
				SQL_addr[i] = SQL_addr[0] + (i * 4); // BYTES_PER_CELL = 4
			}
		}
		Callback_Array(SQL_callbacks[idx], SQL_addr);
		SQL_callbacks[idx][E_CALLBACK_DATA_POINTER] = 0;
	}
	
#endif
