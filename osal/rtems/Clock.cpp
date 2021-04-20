#include "RtemsBasic.h"

#include "../../timemanager/Clock.h"
#include "../../ipc/MutexGuard.h"

#include <rtems/score/todimpl.h>
#include <rtems/rtems/clockimpl.h>

uint16_t Clock::leapSeconds = 0;
MutexIF* Clock::timeMutex = nullptr;

uint32_t Clock::getTicksPerSecond(void){
    rtems_interval ticks_per_second = rtems_clock_get_ticks_per_second();
    return static_cast<uint32_t>(ticks_per_second);
}

ReturnValue_t Clock::setClock(const TimeOfDay_t* time) {
    rtems_time_of_day timeRtems;
    timeRtems.year = time->year;
    timeRtems.month = time->month;
    timeRtems.day = time->day;
    timeRtems.hour = time->hour;
    timeRtems.minute  = time->minute;
    timeRtems.second = time->second;
    timeRtems.ticks = time->usecond * getTicksPerSecond() / 1e6;
    rtems_status_code status = rtems_clock_set(&timeRtems);
    switch(status){
    case RTEMS_SUCCESSFUL:
        return HasReturnvaluesIF::RETURN_OK;
    case RTEMS_INVALID_ADDRESS:
        return HasReturnvaluesIF::RETURN_FAILED;
    case RTEMS_INVALID_CLOCK:
        return HasReturnvaluesIF::RETURN_FAILED;
    default:
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

ReturnValue_t Clock::setClock(const timeval* time) {
    timespec newTime;
    newTime.tv_sec = time->tv_sec;
    if(time->tv_usec < 0) {
        // better returnvalue.
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    newTime.tv_nsec = time->tv_usec * TOD_NANOSECONDS_PER_MICROSECOND;

    ISR_lock_Context context;
    _TOD_Lock();
    _TOD_Acquire(&context);
    Status_Control status = _TOD_Set(&newTime, &context);
    _TOD_Unlock();
    if(status == STATUS_SUCCESSFUL) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    // better returnvalue
    return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t Clock::getClock_timeval(timeval* time) {
    //Callable from ISR
    rtems_status_code status = rtems_clock_get_tod_timeval(time);
    switch(status){
    case RTEMS_SUCCESSFUL:
        return HasReturnvaluesIF::RETURN_OK;
    case RTEMS_NOT_DEFINED:
        return HasReturnvaluesIF::RETURN_FAILED;
    default:
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

ReturnValue_t Clock::getUptime(timeval* uptime) {
    //According to docs.rtems.org for rtems 5 this method is more accurate than rtems_clock_get_ticks_since_boot
    timespec time;
    rtems_status_code status = rtems_clock_get_uptime(&time);
    uptime->tv_sec = time.tv_sec;
    time.tv_nsec = time.tv_nsec / 1000;
    uptime->tv_usec = time.tv_nsec;
    switch(status){
    case RTEMS_SUCCESSFUL:
        return HasReturnvaluesIF::RETURN_OK;
    default:
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

ReturnValue_t Clock::getUptime(uint32_t* uptimeMs) {
    //This counter overflows after 50 days
    *uptimeMs = rtems_clock_get_ticks_since_boot();
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getClock_usecs(uint64_t* time) {
    timeval temp_time;
    rtems_status_code returnValue = rtems_clock_get_tod_timeval(&temp_time);
    *time = ((uint64_t) temp_time.tv_sec * 1000000) + temp_time.tv_usec;
    switch(returnValue){
    case RTEMS_SUCCESSFUL:
        return HasReturnvaluesIF::RETURN_OK;
    default:
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

ReturnValue_t Clock::getDateAndTime(TimeOfDay_t* time) {
    /* For all but the last field, the struct will be filled with the correct values */
    rtems_time_of_day timeRtems;
    rtems_status_code status = rtems_clock_get_tod(&timeRtems);
    switch (status) {
    case RTEMS_SUCCESSFUL: {
        /* The last field now contains the RTEMS ticks of the seconds from 0
        to rtems_clock_get_ticks_per_second() minus one.
        We calculate the microseconds accordingly */
        time->day = timeRtems.day;
        time->hour = timeRtems.hour;
        time->minute = timeRtems.minute;
        time->month = timeRtems.month;
        time->second = timeRtems.second;
        time->usecond = static_cast<float>(timeRtems.ticks) /
                rtems_clock_get_ticks_per_second() * 1e6;
        time->year = timeRtems.year;
        return HasReturnvaluesIF::RETURN_OK;
    }
    case RTEMS_NOT_DEFINED:
        /* System date and time is not set */
        return HasReturnvaluesIF::RETURN_FAILED;
    case RTEMS_INVALID_ADDRESS:
        /* time_buffer is NULL */
        return HasReturnvaluesIF::RETURN_FAILED;
    default:
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

ReturnValue_t Clock::convertTimeOfDayToTimeval(const TimeOfDay_t* from,
        timeval* to) {
    //Fails in 2038..
    rtems_time_of_day timeRtems;
    timeRtems.year = from->year;
    timeRtems.month = from->month;
    timeRtems.day = from->day;
    timeRtems.hour = from->hour;
    timeRtems.minute  = from->minute;
    timeRtems.second = from->second;
    timeRtems.ticks = from->usecond * getTicksPerSecond() / 1e6;
    to->tv_sec = _TOD_To_seconds(&timeRtems);
    to->tv_usec = from->usecond;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::convertTimevalToJD2000(timeval time, double* JD2000) {
    *JD2000 = (time.tv_sec - 946728000. + time.tv_usec / 1000000.) / 24.
            / 3600.;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::convertUTCToTT(timeval utc, timeval* tt) {
    //SHOULDDO: works not for dates in the past (might have less leap seconds)
    if (timeMutex == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    uint16_t leapSeconds;
    ReturnValue_t result = getLeapSeconds(&leapSeconds);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    timeval leapSeconds_timeval = { 0, 0 };
    leapSeconds_timeval.tv_sec = leapSeconds;

    //initial offset between UTC and TAI
    timeval UTCtoTAI1972 = { 10, 0 };

    timeval TAItoTT = { 32, 184000 };

    *tt = utc + leapSeconds_timeval + UTCtoTAI1972 + TAItoTT;

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::setLeapSeconds(const uint16_t leapSeconds_) {
    if(checkOrCreateClockMutex()!=HasReturnvaluesIF::RETURN_OK){
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    MutexGuard helper(timeMutex);


    leapSeconds = leapSeconds_;


    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getLeapSeconds(uint16_t* leapSeconds_) {
    if(timeMutex==nullptr){
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    MutexGuard helper(timeMutex);

    *leapSeconds_ = leapSeconds;

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::checkOrCreateClockMutex(){
    if(timeMutex==nullptr){
        MutexFactory* mutexFactory = MutexFactory::instance();
        if (mutexFactory == nullptr) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        timeMutex = mutexFactory->createMutex();
        if (timeMutex == nullptr) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}
