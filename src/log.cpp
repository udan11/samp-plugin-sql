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

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "mutex.h"

#include "log.h"

logprintf_t logprintf;

int logFile = LOG_ALL;
int logConsole = LOG_WARNING;

Mutex log_mutex;

void log(int level, char *format, ...) {
	if ((level < logFile) && (level < logConsole)) {
		return;
	}
	char prefix[16] = "";
	if (level == LOG_DEBUG) {
		strcpy(prefix, "[debug]");
	} else if (level == LOG_INFO) {
		strcpy(prefix, "[info]");
	} else if (level == LOG_WARNING) {
		strcpy(prefix, "[warning]");
	} else if (level == LOG_ERROR) {
		strcpy(prefix, "[error]");
	}
	va_list args;
	va_start(args, format);
	int len = vsnprintf(NULL, 0, format, args) + 1;
	char *msg = (char*) malloc(sizeof(char) * len);
	if (msg != NULL) {
		time_t rawtime;
		struct tm *timeinfo;
		char timestamp[16];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(timestamp, sizeof(timestamp), "%X", timeinfo);
		vsnprintf(msg, len, format, args);
		log_mutex.lock();
		if (level >= logFile) {
			FILE *file = fopen(LOG_FILE, "a");
			if (file != NULL) {
				fprintf(file, "[%s]%s %s\n", timestamp, prefix, msg);
				fclose(file);
			}
		}
		if (level >= logConsole) {
			logprintf("[plugin.sql]%s %s", prefix, msg);
		}
		log_mutex.unlock();
		free(msg);
	}
	va_end(args);
}
