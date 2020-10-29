#include "IntTestMutex.h"

#include <fsfw/ipc/MutexFactory.h>
#include <unittest/internal/UnittDefinitions.h>

#if defined(hosted)
#include <fsfw/osal/hosted/Mutex.h>
#include <thread>
#include <future>
#endif


void testmutex::testMutex() {
	std::string id = "[testMutex]";
	MutexIF* mutex = MutexFactory::instance()->createMutex();
	auto result = mutex->lockMutex(MutexIF::POLLING);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		unitt::put_error(id);
	}
	// timed_mutex from the C++ library specifies undefined behaviour if
	// the timed mutex is locked twice from the same thread.
#if defined(hosted)
	// hold on, this actually worked ? :-D This calls the function from
	// another thread and stores the returnvalue in a future.
	auto future = std::async(&MutexIF::lockMutex, mutex, 1);
	result = future.get();
#else
	result = mutex->lockMutex(MutexIF::TimeoutType::WAITING, 1);
#endif
	if(result != MutexIF::MUTEX_TIMEOUT) {
		unitt::put_error(id);
	}

	result = mutex->unlockMutex();
	if(result != HasReturnvaluesIF::RETURN_OK) {
		unitt::put_error(id);
	}
	result = mutex->unlockMutex();
	if(result != MutexIF::CURR_THREAD_DOES_NOT_OWN_MUTEX) {
		unitt::put_error(id);
	}
}
