#include "log.h"

logprintf_t logprintf;
int log_level = LOG_DEBUG;

void log(int level, char *format, ...) {
	if (level < log_level) {
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
		char timestamp[TIME_X_LEN];
		strftime(timestamp, sizeof(timestamp), "%X", timeinfo);
		vsprintf(msg, format, args);
		if (level > LOG_INFO) {
			logprintf("plugin.mysql: %s", msg);
		}
		FILE *logFile = fopen(LOG_FILE, "a");
		if (logFile != NULL) {
			fprintf(logFile, "[%s] %s\n", timestamp, msg);
			fclose(logFile);
		}
		free(msg);
	}
	va_end(args);
}