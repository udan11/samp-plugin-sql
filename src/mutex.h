#pragma once

#if ((defined(WIN32)) || (defined(_WIN32)) || (defined(_WIN64)))
	#include "windows.h"
#else
	#include "pthread.h"
#endif

class Mutex {
	public:
		static bool isEnabled;
		static Mutex *getInstance();
		void lock();
		void unlock();
		~Mutex();
	protected:
		Mutex();
	private:
		static Mutex *singleton;
		#ifdef WIN32
			HANDLE handle;
		#else
			pthread_mutex_t handle;
		#endif
};