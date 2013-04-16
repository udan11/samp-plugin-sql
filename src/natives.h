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

class Natives {
	public:
		static cell AMX_NATIVE_CALL mysql_debug(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_connect(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_disconnect(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_set_charset(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL mysql_get_charset(AMX *amx, cell *params);
		static cell AMX_NATIVE_CALL mysql_ping(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_get_stat(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_escape_string(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_query(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_free_result(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_store_result(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_insert_id(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_affected_rows(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_error(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_error_string(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_num_rows(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_num_fields(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_field_name(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_next_row(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_get_field(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_get_field_assoc(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_get_field_int(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_get_field_assoc_int(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_get_field_float(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_get_field_assoc_float(AMX* amx, cell* params);
	private:
		Natives();
		~Natives();
};