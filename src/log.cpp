#include "log.h"

logprintf_t logprintf;
int log_level_file = LOG_ALL, log_level_console = LOG_WARNING;

void log(int level, char *format, ...) {
	if ((level < log_level_file) && (level < log_level_console)) {
		return;
	}
	va_list args;
	va_start(args, format);
	int len = vsnprintf(NULL, 0, format, args);
	char *msg = (char*) malloc(len + 1);
	if (msg != NULL) {
		time_t rawtime;
		struct tm *timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		char timestamp[16];
		strftime(timestamp, sizeof(timestamp), "%X", timeinfo);
		vsprintf(msg, format, args);
		if (level >= log_level_file) {
			FILE *logFile = fopen(LOG_FILE, "a");
			if (logFile != NULL) {
				fprintf(logFile, "[%s] %s\n", timestamp, msg);
				fclose(logFile);
			}
		}
		if (level >= log_level_console) {
			logprintf("plugin.mysql: %s", msg);
		}
		free(msg);
	}
	va_end(args);
}