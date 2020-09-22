#include "Stopwatch.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include <iomanip>

Stopwatch::Stopwatch(bool displayOnDestruction,
        StopwatchDisplayMode displayMode): displayOnDestruction(
        displayOnDestruction), displayMode(displayMode) {
    // Measures start time on initialization.
    Clock::getUptime(&startTime);
}

void Stopwatch::start() {
    Clock::getUptime(&startTime);
}

dur_millis_t Stopwatch::stop(bool display) {
    stopInternal();
    if(display) {
        this->display();
    }
    return elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000;
}

double Stopwatch::stopSeconds() {
    stopInternal();
    return timevalOperations::toDouble(elapsedTime);
}

void Stopwatch::display() {
    if(displayMode == StopwatchDisplayMode::MILLIS) {
        sif::info << "Stopwatch: Operation took " << (elapsedTime.tv_sec * 1000 +
                elapsedTime.tv_usec / 1000) << " milliseconds" << std::endl;
    }
    else if(displayMode == StopwatchDisplayMode::SECONDS) {
        sif::info <<"Stopwatch: Operation took "  << std::setprecision(3)
             << std::fixed << timevalOperations::toDouble(elapsedTime)
             << " seconds" << std::endl;
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
	timeval endTime;
	Clock::getUptime(&endTime);
    elapsedTime = endTime - startTime;
}
