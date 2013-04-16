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

#include "mutex.h"

Mutex *Mutex::singleton = NULL;
bool Mutex::isEnabled = false;

Mutex::Mutex() {
#ifdef WIN32
	handle = CreateMutex(NULL, FALSE, "samp_plugin_mysql");
#else
	//handle = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&handle, &attr);
#endif
	isEnabled = true;
}

Mutex::~Mutex() {
#ifdef WIN32
	CloseHandle(handle);
#else
	pthread_mutex_destroy(&handle);
#endif
	isEnabled = false;
	singleton = NULL;
}

Mutex *Mutex::getInstance() {
	if (singleton == NULL) {
		singleton = new Mutex();
	}
	return singleton;
}

void Mutex::lock() {
	if (isEnabled) {
	#ifdef WIN32
		WaitForSingleObject(handle, INFINITE);
	#else
		pthread_mutex_lock(&handle);
	#endif
	}
}

void Mutex::unlock() {
	if (isEnabled) {
	#ifdef WIN32
		ReleaseMutex(handle);
	#else
		pthread_mutex_unlock(&handle);
	#endif
	}
}