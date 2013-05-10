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

#include "main.h"

class Natives {
	public:
		static cell AMX_NATIVE_CALL sql_debug(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_connect(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_disconnect(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_set_charset(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_get_charset(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_ping(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_get_stat(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_escape_string(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_query(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_free_result(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_store_result(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_insert_id(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_affected_rows(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_error(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_error_string(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_num_rows(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_num_fields(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_next_result(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_field_name(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_next_row(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_get_field(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_get_field_assoc(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_get_field_int(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_get_field_assoc_int(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_get_field_float(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL sql_get_field_assoc_float(AMX* amx, cell* params);
	private:
		Natives();
		~Natives();
};