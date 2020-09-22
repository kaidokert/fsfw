#include "Mutex.h"
#include "PeriodicTask.h"

#include "../../ipc/MutexFactory.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../tasks/ExecutableObjectIF.h"

#include <thread>
#include <chrono>

#if defined(WIN32)
#include <windows.h>
#elif defined(LINUX)
#include <pthread.h>
#endif

PeriodicTask::PeriodicTask(const char *name, TaskPriority setPriority,
		TaskStackSize setStack, TaskPeriod setPeriod,
		void (*setDeadlineMissedFunc)()) :
		started(false), taskName(name), period(setPeriod),
		deadlineMissedFunc(setDeadlineMissedFunc) {
    // It is propably possible to set task priorities by using the native
    // task handles for Windows / Linux
	mainThread = std::thread(&PeriodicTask::taskEntryPoint, this, this);
#if defined(WIN32)
    /* List of possible priority classes:
     * https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/
     * nf-processthreadsapi-setpriorityclass
     * And respective thread priority numbers:
     * https://docs.microsoft.com/en-us/windows/
     * win32/procthread/scheduling-priorities */
    int result = SetPriorityClass(
            reinterpret_cast<HANDLE>(mainThread.native_handle()),
            ABOVE_NORMAL_PRIORITY_CLASS);
    if(result != 0) {
        sif::error << "PeriodicTask: Windows SetPriorityClass failed with code "
                << GetLastError() << std::endl;
    }
    result = SetThreadPriority(
            reinterpret_cast<HANDLE>(mainThread.native_handle()),
            THREAD_PRIORITY_NORMAL);
    if(result != 0) {
        sif::error << "PeriodicTask: Windows SetPriorityClass failed with code "
                << GetLastError() << std::endl;
    }
#elif defined(LINUX)
    // we can just copy and paste the code from linux here.
#endif
}

PeriodicTask::~PeriodicTask(void) {
	//Do not delete objects, we were responsible for ptrs only.
	terminateThread = true;
	if(mainThread.joinable()) {
		mainThread.join();
	}
	delete this;
}

void PeriodicTask::taskEntryPoint(void* argument) {
	PeriodicTask *originalTask(reinterpret_cast<PeriodicTask*>(argument));


	if (not originalTask->started) {
		// we have to suspend/block here until the task is started.
		// if semaphores are implemented, use them here.
		std::unique_lock<std::mutex> lock(initMutex);
		initCondition.wait(lock);
	}

	this->taskFunctionality();
	sif::debug << "PeriodicTask::taskEntryPoint: "
			"Returned from taskFunctionality." << std::endl;
}

ReturnValue_t PeriodicTask::startTask() {
	started = true;

	// Notify task to start.
	std::lock_guard<std::mutex> lock(initMutex);
	initCondition.notify_one();

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PeriodicTask::sleepFor(uint32_t ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	return HasReturnvaluesIF::RETURN_OK;
}

void PeriodicTask::taskFunctionality() {
	std::chrono::milliseconds periodChrono(static_cast<uint32_t>(period*1000));
	auto currentStartTime {
	    std::chrono::duration_cast<std::chrono::milliseconds>(
	    std::chrono::system_clock::now().time_since_epoch())
	};
	auto nextStartTime{ currentStartTime };

	/* Enter the loop that defines the task behavior. */
	for (;;) {
		if(terminateThread.load()) {
			break;
		}
		for (ObjectList::iterator it = objectList.begin();
				it != objectList.end(); ++it) {
			(*it)->performOperation();
		}
		if(not delayForInterval(&currentStartTime, periodChrono)) {
			sif::warning << "PeriodicTask: " << taskName <<
					" missed deadline!\n" << std::flush;
			if(deadlineMissedFunc != nullptr) {
				this->deadlineMissedFunc();
			}
		}
	}
}

ReturnValue_t PeriodicTask::addComponent(object_id_t object) {
	ExecutableObjectIF* newObject = objectManager->get<ExecutableObjectIF>(
			object);
	if (newObject == nullptr) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	objectList.push_back(newObject);
	return HasReturnvaluesIF::RETURN_OK;
}

uint32_t PeriodicTask::getPeriodMs() const {
	return period * 1000;
}

bool PeriodicTask::delayForInterval(chron_ms* previousWakeTimeMs,
        const chron_ms interval) {
    bool shouldDelay = false;
    //Get current wakeup time
    auto currentStartTime =
            std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    /* Generate the tick time at which the task wants to wake. */
    auto nextTimeToWake_ms = (*previousWakeTimeMs) + interval;

    if (currentStartTime < *previousWakeTimeMs) {
        /* The tick count has overflowed since this function was
         lasted called.  In this case the only time we should ever
         actually delay is if the wake time has also    overflowed,
         and the wake time is greater than the tick time.  When this
         is the case it is as if neither time had overflowed. */
        if ((nextTimeToWake_ms < *previousWakeTimeMs)
                && (nextTimeToWake_ms > currentStartTime)) {
            shouldDelay = true;
        }
    } else {
        /* The tick time has not overflowed.  In this case we will
         delay if either the wake time has overflowed, and/or the
         tick time is less than the wake time. */
        if ((nextTimeToWake_ms < *previousWakeTimeMs)
                || (nextTimeToWake_ms >  currentStartTime)) {
            shouldDelay = true;
        }
    }

    /* Update the wake time ready for the next call. */

    (*previousWakeTimeMs) =  nextTimeToWake_ms;

    if (shouldDelay) {
        auto sleepTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                nextTimeToWake_ms - currentStartTime);
        std::this_thread::sleep_for(sleepTime);
        return true;
    }
    //We are shifting the time in case the deadline was missed like rtems
    (*previousWakeTimeMs) = currentStartTime;
    return false;

}
