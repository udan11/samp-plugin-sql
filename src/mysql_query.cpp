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

#include "mysql_query.h"

void free_query(struct mysql_query *&query) {
	free(query->query);
	free(query->callback);
	free(query->format);
	query->params_c.clear();
	for (int i = 0, size = query->params_s.size(); i != size; ++i) {
		free(query->params_s[i]);
	}
	query->params_s.clear();
	if (query->result != NULL) {
		mysql_free_result(query->result);
	}
	for (int i = 0, size = query->field_names.size(); i != size; ++i) {
		free(query->field_names[i].first);
	}
	query->field_names.clear();
	for (int i = 0, size = query->cache.size(); i != size; ++i) {
		for (int j = 0, size = query->cache[i].size(); j != size; ++j) {
			free(query->cache[i][j].first);
		}
		query->cache[i].clear();
	}
	query->cache.clear();
	free(query);
}

int execute_query_callback(struct mysql_query *query) {
	AMX *amx = handlers[query->handler]->amx;
	cell ret, amx_addr = -1;
	int funcidx;
	if (query->error == 0) {
		if (!amx_FindPublic(amx, query->callback, &funcidx)) {
			int c_idx = query->params_c.size() - 1, s_idx = query->params_s.size() - 1;
			for (int i = strlen(query->format) - 1; i >= 0; --i) {
				switch (query->format[i]) {
					case 'c':
					case 'C':
					case 'd':
					case 'D':
					case 'i':
					case 'I':
					case 'f':
					case 'F':
						amx_Push(amx, query->params_c[c_idx--]);
						break;
					case 'r':
					case 'R':
						amx_Push(amx, query->id);
						break;
					case 's':
					case 'S':
						if (amx_addr < NULL) {
							amx_addr = NULL;
						}
						amx_PushString(amx, &amx_addr, NULL, query->params_s[s_idx--], NULL, NULL);
						break;
				}
			}
			amx_Exec(amx, &ret, funcidx);
		}
	} else {
		if (!amx_FindPublic(amx, QUERY_ERROR_CALLBACK, &funcidx)) {
			amx_addr = NULL;
			amx_PushString(amx, &amx_addr, NULL, query->callback, NULL, NULL);
			amx_PushString(amx, &amx_addr, NULL, query->query, NULL, NULL);
			amx_PushString(amx, &amx_addr, NULL, query->error_msg, NULL, NULL);
			amx_Push(amx, (cell) query->error);
			amx_Push(amx, (cell) query->handler);
			amx_Exec(amx, &ret, funcidx);
		}
	}
	if (amx_addr >= NULL) {
		amx_Release(amx, amx_addr);
	}
	return (int) ret;
}