#include "testMutex.h"

#include <fsfw/ipc/MutexFactory.h>

#include "fsfw/platform.h"
#include "fsfw_tests/internal/UnittDefinitions.h"

#if defined PLATFORM_WIN || defined PLATFORM_UNIX
#include <future>
#include <thread>

#include "fsfw/osal/host/Mutex.h"
#endif

void testmutex::testMutex() {
  std::string id = "[testMutex]";
  MutexIF* mutex = MutexFactory::instance()->createMutex();
  auto result = mutex->lockMutex(MutexIF::TimeoutType::POLLING);
  if (result != returnvalue::OK) {
    unitt::put_error(id);
  }
  // timed_mutex from the C++ library specifies undefined behaviour if
  // the timed mutex is locked twice from the same thread.
  // TODO: we should pass a define like FSFW_OSAL_HOST to the build.
#if defined PLATFORM_WIN || defined PLATFORM_UNIX
  // This calls the function from
  // another thread and stores the returnvalue in a future.
  auto future = std::async(&MutexIF::lockMutex, mutex, MutexIF::TimeoutType::WAITING, 1);
  result = future.get();
#else
  result = mutex->lockMutex(MutexIF::TimeoutType::WAITING, 1);
#endif
  if (result != MutexIF::MUTEX_TIMEOUT) {
    unitt::put_error(id);
  }

  result = mutex->unlockMutex();
  if (result != returnvalue::OK) {
    unitt::put_error(id);
  }

#if !defined PLATFORM_WIN && !defined PLATFORM_UNIX
  result = mutex->unlockMutex();
  if (result != MutexIF::CURR_THREAD_DOES_NOT_OWN_MUTEX) {
    unitt::put_error(id);
  }
#endif
}
