#include "main.h"

void **ppPluginData;
extern void *pAMXFunctions;

bool running = false;
int last_handler = 1, last_query = 1;
std::map<int, class MySQL_Handler*> handlers;
std::map<int, struct mysql_query*> queries;

#ifdef WIN32
	DWORD __stdcall ProcessQueryThread(LPVOID lpParam);
#else
	void *ProcessQueryThread(void *lpParam);
#endif

const AMX_NATIVE_INFO NATIVES[] = {
	{"mysql_debug", Natives::mysql_debug},
	{"mysql_connect", Natives::mysql_connect},
	{"mysql_disconnect", Natives::mysql_disconnect},
	{"mysql_set_charset", Natives::mysql_set_charset},
	{"mysql_get_charset", Natives::mysql_get_charset},
	{"mysql_ping", Natives::mysql_ping},
	{"mysql_get_stat", Natives::mysql_get_stat},
	{"mysql_escape_string", Natives::mysql_escape_string},
	{"mysql_query", Natives::mysql_query},
	{"mysql_store_result", Natives::mysql_store_result},
	{"mysql_free_result", Natives::mysql_free_result},
	{"mysql_insert_id", Natives::mysql_insert_id},
	{"mysql_affected_rows", Natives::mysql_affected_rows},
	{"mysql_error", Natives::mysql_error},
	{"mysql_error_string", Natives::mysql_error_string},
	{"mysql_num_rows", Natives::mysql_num_rows},
	{"mysql_num_fields", Natives::mysql_num_fields},
	{"mysql_field_name", Natives::mysql_field_name},
	{"mysql_next_row", Natives::mysql_next_row},
	{"mysql_get_field", Natives::mysql_get_field},
	{"mysql_get_field_assoc", Natives::mysql_get_field_assoc},
	{"mysql_get_field_int", Natives::mysql_get_field_int},
	{"mysql_get_field_assoc_int", Natives::mysql_get_field_assoc_int},
	{"mysql_get_field_float", Natives::mysql_get_field_float},
	{"mysql_get_field_assoc_float", Natives::mysql_get_field_assoc_float},
	{NULL, NULL}
};

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t) ppData[PLUGIN_DATA_LOGPRINTF];
	if (mysql_library_init(0, NULL, NULL)) {
		logprintf("  >> Coudln't initalize the MySQL library (libmysql). It's probably missing.");
		exit(0);
		return 0;
	}
	running = true;
#ifdef WIN32
	HANDLE threadHandle;
	DWORD dwThreadId = 0;
	threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ProcessQueryThread, NULL, 0, &dwThreadId);
	CloseHandle(threadHandle);
#else
	pthread_t threadHandle;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&threadHandle, &attr, &ProcessQueryThread, NULL);
#endif
	Mutex::getInstance();
	logprintf("  >> MySQL plugin " PLUGIN_VERSION " successfully loaded.");
	return 1;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) {
	return amx_Register(amx, NATIVES, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) {
	Mutex::getInstance()->lock();
	for (std::map<int, class MySQL_Handler*>::iterator it = handlers.begin(), next = it; it != handlers.end(); it = next) {
		++next;
		MySQL_Handler *handler = it->second;
		if (handler->amx == amx) {
			delete handler;
			free(handler);
		}
	}
	Mutex::getInstance()->unlock();
	return AMX_ERR_NONE;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
	log(LOG_INFO, "Plugin is being unloaded...");
	running = false;
	Mutex::getInstance()->lock();
	for (std::map<int, class MySQL_Handler*>::iterator it = handlers.begin(), next = it; it != handlers.end(); it = next) {
		++next;
		MySQL_Handler *handler = it->second;
		delete handler;
		handlers.erase(it);
	}
	handlers.clear();
	for (std::map<int, struct mysql_query*>::iterator it = queries.begin(), next = it; it != queries.end(); it = next) {
		++next;
		struct mysql_query *query = it->second;
		free_query(query);
		queries.erase(it);
	}
	queries.clear();
	Mutex::getInstance()->unlock();
	delete Mutex::getInstance();
	log(LOG_INFO, "Plugin succesfully unloaded!");
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() {
	Mutex::getInstance()->lock();
	for (std::map<int, struct mysql_query*>::iterator it = queries.begin(), next = it; it != queries.end(); it = next) {
		++next;
		struct mysql_query *query = it->second;
		if ((query->flags & QUERY_THREADED) && (query->status == QUERY_STATUS_EXECUTED)) {
			log(LOG_DEBUG, "ProccessTick(): Executing query callback (query->id = %d, query->error = %d, query->callback = %s)...", query->id, query->error, query->callback);
			query->status = QUERY_STATUS_PROCESSED;
			execute_query_callback(query);
		}
		if ((!is_valid_handler(query->handler)) || (query->status == QUERY_STATUS_PROCESSED)) {
			log(LOG_DEBUG, "ProccessTick(): Erasing query (query->id = %d)...", query->id);
			free_query(query);
			queries.erase(it);
		}
	}
	Mutex::getInstance()->unlock();
}

#ifdef WIN32
	DWORD __stdcall ProcessQueryThread(LPVOID lpParam)
#else
	void *ProcessQueryThread(void *lpParam)
#endif
{
	while (running) {
		Mutex::getInstance()->lock();
		for (std::map<int, struct mysql_query*>::iterator it = queries.begin(), next = it; it != queries.end(); it = next) {
			++next;
			struct mysql_query *query = it->second;
			if ((query->flags & QUERY_THREADED) && (query->status == QUERY_STATUS_NONE)) {
				log(LOG_DEBUG, "ProcessQueryThread(): Executing query (query->id = %d, query->query = %d)...", query->id, query->query);
				handlers[query->handler]->execute_query(query);
			}
		}
		Mutex::getInstance()->unlock();
		SLEEP(50);
	}
	return 0;
}