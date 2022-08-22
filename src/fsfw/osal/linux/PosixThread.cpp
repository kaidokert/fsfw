#include "fsfw/osal/linux/PosixThread.h"

#include <errno.h>

#include <cstring>

#include "fsfw/osal/linux/unixUtility.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

PosixThread::PosixThread(const char* name_, int priority_, size_t stackSize_)
    : thread(0), priority(priority_), stackSize(stackSize_) {
  name[0] = '\0';
  std::strncat(name, name_, PTHREAD_MAX_NAMELEN - 1);
}

PosixThread::~PosixThread() {
  // No deletion and no free of Stack Pointer
}

ReturnValue_t PosixThread::sleep(uint64_t ns) {
  // TODO sleep might be better with timer instead of sleep()
  timespec time;
  time.tv_sec = ns / 1000000000;
  time.tv_nsec = ns - time.tv_sec * 1e9;

  // Remaining Time is not set here
  int status = nanosleep(&time, NULL);
  if (status != 0) {
    switch (errno) {
      case EINTR:
        // The nanosleep() function was interrupted by a signal.
        return returnvalue::FAILED;
      case EINVAL:
        // The rqtp argument specified a nanosecond value less than zero or
        //  greater than or equal to 1000 million.
        return returnvalue::FAILED;
      default:
        return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

void PosixThread::suspend() {
  // Wait for SIGUSR1
  int caughtSig = 0;
  sigset_t waitSignal;
  sigemptyset(&waitSignal);
  sigaddset(&waitSignal, SIGUSR1);
  sigwait(&waitSignal, &caughtSig);
  if (caughtSig != SIGUSR1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "FixedTimeslotTask::suspend: Unknown Signal received: " << caughtSig << std::endl;
#else
    sif::printError("FixedTimeslotTask::suspend: Unknown Signal received: %d\n", caughtSig);
#endif
  }
}

void PosixThread::resume() {
  /* Signal the thread to start. Makes sense to call kill to start or? ;)
   *
   *	According to Posix raise(signal) will call pthread_kill(pthread_self(), sig),
   *	but as the call must be done from the thread itsself this is not possible here
   */
  pthread_kill(thread, SIGUSR1);
}

bool PosixThread::delayUntil(uint64_t* const prevoiusWakeTime_ms, const uint64_t delayTime_ms) {
  uint64_t nextTimeToWake_ms;
  bool shouldDelay = false;
  // Get current Time
  const uint64_t currentTime_ms = getCurrentMonotonicTimeMs();
  /* Generate the tick time at which the task wants to wake. */
  nextTimeToWake_ms = (*prevoiusWakeTime_ms) + delayTime_ms;

  if (currentTime_ms < *prevoiusWakeTime_ms) {
    /* The tick count has overflowed since this function was
            lasted called.  In this case the only time we should ever
            actually delay is if the wake time has also overflowed,
            and the wake time is greater than the tick time. When this
            is the case it is as if neither time had overflowed. */
    if ((nextTimeToWake_ms < *prevoiusWakeTime_ms) && (nextTimeToWake_ms > currentTime_ms)) {
      shouldDelay = true;
    }
  } else {
    /* The tick time has not overflowed. In this case we will
            delay if either the wake time has overflowed, and/or the
            tick time is less than the wake time. */
    if ((nextTimeToWake_ms < *prevoiusWakeTime_ms) || (nextTimeToWake_ms > currentTime_ms)) {
      shouldDelay = true;
    }
  }

  /* Update the wake time ready for the next call. */

  (*prevoiusWakeTime_ms) = nextTimeToWake_ms;

  if (shouldDelay) {
    uint64_t sleepTime = nextTimeToWake_ms - currentTime_ms;
    PosixThread::sleep(sleepTime * 1000000ull);
    return true;
  }
  // We are shifting the time in case the deadline was missed like rtems
  (*prevoiusWakeTime_ms) = currentTime_ms;
  return false;
}

uint64_t PosixThread::getCurrentMonotonicTimeMs() {
  timespec timeNow;
  clock_gettime(CLOCK_MONOTONIC_RAW, &timeNow);
  uint64_t currentTime_ms = (uint64_t)timeNow.tv_sec * 1000 + timeNow.tv_nsec / 1000000;

  return currentTime_ms;
}

void PosixThread::createTask(void* (*fnc_)(void*), void* arg_) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  // sif::debug << "PosixThread::createTask" << std::endl;
#endif
  /*
   * The attr argument points to a pthread_attr_t structure whose contents
     are used at thread creation time to determine attributes for the new
     thread; this structure is initialized using pthread_attr_init(3) and
     related functions.  If attr is NULL, then the thread is created with
     default attributes.
   */
  pthread_attr_t attributes;
  int status = pthread_attr_init(&attributes);
  if (status != 0) {
    utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "pthread_attr_init");
  }
  void* stackPointer;
  status = posix_memalign(&stackPointer, sysconf(_SC_PAGESIZE), stackSize);
  if (status != 0) {
    if (errno == ENOMEM) {
      size_t stackMb = stackSize / 10e6;
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "PosixThread::createTask: Insufficient memory for"
                    " the requested "
                 << stackMb << " MB" << std::endl;
#else
      sif::printError(
          "PosixThread::createTask: Insufficient memory for "
          "the requested %lu MB\n",
          static_cast<unsigned long>(stackMb));
#endif
      utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "ENOMEM");
    } else if (errno == EINVAL) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "PosixThread::createTask: Wrong alignment argument!" << std::endl;
#else
      sif::printError("PosixThread::createTask: Wrong alignment argument!\n");
#endif
      utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "EINVAL");
    }
    return;
  }

  status = pthread_attr_setstack(&attributes, stackPointer, stackSize);
  if (status != 0) {
    utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "pthread_attr_setstack");
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "Make sure the specified stack size is valid and is "
                    "larger than the minimum allowed stack size."
                 << std::endl;
#else
    sif::printWarning(
        "Make sure the specified stack size is valid and is "
        "larger than the minimum allowed stack size.\n");
#endif
  }

  status = pthread_attr_setinheritsched(&attributes, PTHREAD_EXPLICIT_SCHED);
  if (status != 0) {
    utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "pthread_attr_setinheritsched");
  }
#ifndef FSFW_USE_REALTIME_FOR_LINUX
#error "Please define FSFW_USE_REALTIME_FOR_LINUX with either 0 or 1"
#endif
#if FSFW_USE_REALTIME_FOR_LINUX == 1
  // FIFO -> This needs root privileges for the process
  status = pthread_attr_setschedpolicy(&attributes, SCHED_FIFO);
  if (status != 0) {
    utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "pthread_attr_setschedpolicy");
  }

  sched_param scheduleParams;
  scheduleParams.__sched_priority = priority;
  status = pthread_attr_setschedparam(&attributes, &scheduleParams);
  if (status != 0) {
    utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "pthread_attr_setschedparam");
  }
#endif
  // Set Signal Mask for suspend until startTask is called
  sigset_t waitSignal;
  sigemptyset(&waitSignal);
  sigaddset(&waitSignal, SIGUSR1);
  status = pthread_sigmask(SIG_BLOCK, &waitSignal, NULL);
  if (status != 0) {
    utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "pthread_sigmask");
  }

  status = pthread_create(&thread, &attributes, fnc_, arg_);
  if (status != 0) {
    utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "pthread_create");
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "For FSFW_USE_REALTIME_FOR_LINUX == 1 make sure to call "
               << "\"all sudo setcap 'cap_sys_nice=eip'\" on the application or set "
                  "/etc/security/limit.conf"
               << std::endl;
#else
    sif::printError(
        "For FSFW_USE_REALTIME_FOR_LINUX == 1 make sure to call "
        "\"all sudo setcap 'cap_sys_nice=eip'\" on the application or set "
        "/etc/security/limit.conf\n");
#endif
  }

  status = pthread_setname_np(thread, name);
  if (status != 0) {
    utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "pthread_setname_np");
    if (status == ERANGE) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "PosixThread::createTask: Task name length longer"
                      " than 16 chars. Truncating.."
                   << std::endl;
#else
      sif::printWarning(
          "PosixThread::createTask: Task name length longer"
          " than 16 chars. Truncating..\n");
#endif
      name[15] = '\0';
      status = pthread_setname_np(thread, name);
      if (status != 0) {
        utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "pthread_setname_np");
      }
    }
  }

  status = pthread_attr_destroy(&attributes);
  if (status != 0) {
    utility::printUnixErrorGeneric(CLASS_NAME, "createTask", "pthread_attr_destroy");
  }
}
