#include "mysql_query.h"

void free_query(struct mysql_query *&query) {
	free(query->query);
	free(query->callback);
	free(query->format);
	query->params_i.clear();
	query->params_f.clear();
	for (int i = 0, size = query->params_s.size(); i != size; ++i) {
		free(query->params_s[i]);
	}
	query->params_s.clear();
	if (query->result != NULL) {
		mysql_free_result(query->result);
	}
	for (int i = 0, size = query->field_names.size(); i != size; ++i) {
		free(query->field_names[i]);
	}
	query->field_names.clear();
	for (int i = 0, size = query->cache.size(); i != size; ++i) {
		for (int j = 0, size = query->cache[i].size(); j != size; ++j) {
			free(query->cache[i][j]);
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
			int i_idx = query->params_i.size() - 1, f_idx = query->params_f.size() - 1, s_idx = query->params_s.size() - 1;
			for (int i = strlen(query->format) - 1; i >= 0; --i) {
				switch (query->format[i]) {
					case 'c':
					case 'C':
					case 'd':
					case 'D':
					case 'i':
					case 'I':
						amx_Push(amx, query->params_i[i_idx--]);
						break;
					case 'f': 
					case 'F':
						amx_Push(amx, amx_ftoc(query->params_f[f_idx--]));
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