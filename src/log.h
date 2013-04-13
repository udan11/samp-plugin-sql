#pragma once

#include "main.h"

#define LOG_FILE						"mysql_log.txt"

#define LOG_ALL							0
#define LOG_DEBUG						1
#define LOG_INFO						2
#define LOG_WARNING						3
#define LOG_ERROR						4
#define LOG_NONE						5

typedef void (*logprintf_t) (char* format, ...);
extern logprintf_t logprintf;
extern int log_level_file, log_level_console;

extern void log(int level, char *format, ...);