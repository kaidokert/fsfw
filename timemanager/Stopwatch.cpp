/**
 * @file Stopwatch.cpp
 *
 * @date 08.04.2020
 */

#include <framework/timemanager/Stopwatch.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <iomanip>

Stopwatch::Stopwatch(bool displayOnDestruction,
        StopwatchDisplayMode displayMode, uint8_t precision):
        displayOnDestruction(displayOnDestruction), outputPrecision(precision) {
    // Measures start time on initialization.
    Clock::getUptime(&startTime);
}

void Stopwatch::start() {
    startTime = Clock::getUptime();
}

ms_normal_t Stopwatch::stop() {
    stopInternal();
    return elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000;
}

ms_double_t Stopwatch::stopPrecise() {
    stopInternal();
    return elapsedTimeMsDouble;
}


void Stopwatch::display() {
    if(displayMode == StopwatchDisplayMode::MS_DOUBLE) {
        info << "Stopwatch: Operation took "
             << std::setprecision(outputPrecision) << elapsedTimeMsDouble
             << " milliseconds" << std::endl;
    }
    else {
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

void Stopwatch::setDisplayMode(StopwatchDisplayMode displayMode) {
    this->displayMode = displayMode;
}

StopwatchDisplayMode Stopwatch::getDisplayMode() const {
    return displayMode;
}



void Stopwatch::stopInternal() {
    elapsedTime = Clock::getUptime() - startTime;
    elapsedTimeMsDouble = timevalOperations::toDouble(elapsedTime) * 1000.0;
}
