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