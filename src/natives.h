#pragma once

#include "main.h"

class Natives {
	public:
		static cell AMX_NATIVE_CALL mysql_debug(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_connect(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_disconnect(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_ping(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_get_info(AMX* amx, cell* params);
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
		static cell AMX_NATIVE_CALL mysql_get_field(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_fetch_num(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_fetch_assoc(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_fetch_num_int(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_fetch_assoc_int(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_fetch_num_float(AMX* amx, cell* params);
		static cell AMX_NATIVE_CALL mysql_fetch_assoc_float(AMX* amx, cell* params);
	private:
		Natives();
		~Natives();
};