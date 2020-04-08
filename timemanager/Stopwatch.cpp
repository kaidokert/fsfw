/**
 * @file Stopwatch.cpp
 *
 * @date 08.04.2020
 */

#include <framework/timemanager/Stopwatch.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

Stopwatch::Stopwatch(bool displayOnDestruction, DisplayMode displayMode):
        displayOnDestruction(displayOnDestruction) {
    Clock::getUptime(&startTime);
    stopwatchState = StopwatchState::STARTED;
}

void Stopwatch::start() {
    startTime = Clock::getUptime();
}

ms_normal_t Stopwatch::stop() {
    elapsedTime = Clock::getUptime() - startTime;
    int32_t elapsedTimeMs = elapsedTime.tv_sec * 1000 +
            elapsedTime.tv_usec/1000;
    if(elapsedTimeMs < 0) {
        error << "Stopwatch: Measured time negative!";
        return INVALID_TIME;
    }
    else {
        return static_cast<ms_normal_t>(elapsedTimeMs);
    }
}

ms_double_t Stopwatch::stopPrecise() {
    elapsedTime = Clock::getUptime() - startTime;
    return timevalOperations::toDouble(elapsedTime) * 1000.0;
}


void Stopwatch::display() {
    if(displayMode == DisplayMode::MS_FLOAT) {
        info << "Stopwatch: Operation took " <<
                elapsedTimeMs << " milliseconds" << std::endl;
    }
    else {
        timeval elapsedTime = stopTime - startTime;
        info << "Stopwatch: Operation took " << elapsedTime.tv_sec * 1000 +
                elapsedTime.tv_usec * 1000 << " milliseconds";
    }
}


Stopwatch::~Stopwatch() {
    if(displayOnDestruction) {
        stopInternal();
        display();
    }
}

void Stopwatch::stopInternal() {
    elapsedTime = Clock::getUptime() - startTime;
    elapsedTimeMs = timevalOperations::toDouble(elapsedTime) * 1000.0;
}
