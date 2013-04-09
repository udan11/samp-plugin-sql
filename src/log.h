#pragma once

#include "main.h"

#define LOG_FILE						"mysql_log.txt"

#define LOG_DEBUG						1
#define LOG_INFO						2
#define LOG_WARNING						3
#define LOG_ERROR						4

#define TIME_X_LEN						16

typedef void (*logprintf_t) (char* format, ...);
extern logprintf_t logprintf;
extern int log_level;

extern void log(int level, char *format, ...);