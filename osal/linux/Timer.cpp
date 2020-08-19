#include "../../serviceinterface/ServiceInterfaceStream.h"
#include <errno.h>
#include "Timer.h"

Timer::Timer() {
	sigevent sigEvent;
	sigEvent.sigev_notify = SIGEV_NONE;
	sigEvent.sigev_signo = 0;
	sigEvent.sigev_value.sival_ptr = &timerId;
	int status = timer_create(CLOCK_MONOTONIC, &sigEvent, &timerId);
	if(status!=0){
		sif::error << "Timer creation failed with: " << status <<
				" errno: " << errno << std::endl;
	}
}

Timer::~Timer() {
	timer_delete(timerId);
}

int Timer::setTimer(uint32_t intervalMs) {
	itimerspec timer;
	timer.it_value.tv_sec = intervalMs / 1000;
	timer.it_value.tv_nsec = (intervalMs * 1000000) % (1000000000);
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_nsec = 0;
	return timer_settime(timerId, 0, &timer, NULL);
}


int Timer::getTimer(uint32_t* remainingTimeMs){
	itimerspec timer;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_nsec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_nsec = 0;
	int status = timer_gettime(timerId, &timer);

	*remainingTimeMs = timer.it_value.tv_sec * 1000 + timer.it_value.tv_nsec / 1000000;

	return status;
}
