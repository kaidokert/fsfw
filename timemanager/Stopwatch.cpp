#include <framework/timemanager/Stopwatch.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <iomanip>

Stopwatch::Stopwatch(bool displayOnDestruction,
        StopwatchDisplayMode displayMode): displayMode(displayMode),
        displayOnDestruction(displayOnDestruction) {
    // Measures start time on initialization.
    startTime = Clock::getUptime();
}

void Stopwatch::start() {
    startTime = Clock::getUptime();
}

millis_t Stopwatch::stop() {
    stopInternal();
    return elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000;
}

seconds_t Stopwatch::stopSeconds() {
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
    elapsedTime = Clock::getUptime() - startTime;
}
