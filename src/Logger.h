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

#pragma once

//#include "Mutex.h"

#define LOG_FILE						"sql_log.txt"

#define LOG_ALL							0
#define LOG_DEBUG						1
#define LOG_INFO						2
#define LOG_WARNING						3
#define LOG_ERROR						4
#define LOG_NONE						5

typedef void (*logprintf_t) (char *format, ...);

class Logger {

	/**
	 * Exported AMX natives.
	 */
	public:

		/**
		 * This logging system is not thread-safe. Using it in multiple threads
		 * requires a mutex, which decreases the performance significantly.
		 *
		Mutex mutex;
		 */

		/**
		 * SA-MP server's internal logging method.
		 */
		static logprintf_t logprintf;

		/**
		 * The minimum level of the messages that are saved in `LOG_FILE`.
		 * By default, it is set to `LOG_ALL`.
		 */
		static int fileLevel;

		/**
		 * The minimum level of the messages that are printed to the console.
		 * By default, it is set to `LOG_WARNING`.
		 */
		static int consoleLevel;

		/**
		 * Formats and outputs the log message.
		 * @param level
		 * @param format
		 */
		static void log(int level, char *format, ...);

	/**
	 * Static class.
	 */
	private:
		
		/**
		 * Constructor.
		 */
		Logger();

		/**
		 * Destructor.
		 */
		~Logger();
};
