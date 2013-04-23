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

#define LOG_FILE						"mysql_log.txt"

#define LOG_ALL							0
#define LOG_DEBUG						1
#define LOG_INFO						2
#define LOG_WARNING						3
#define LOG_ERROR						4
#define LOG_NONE						5

typedef void (*logprintf_t) (char *format, ...);
extern logprintf_t logprintf;
extern int log_level_file, log_level_console;

extern void log(int level, char *format, ...);