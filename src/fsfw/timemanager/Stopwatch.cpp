#include "fsfw/timemanager/Stopwatch.h"

#include "fsfw/serviceinterface/ServiceInterface.h"

#if FSFW_CPP_OSTREAM_ENABLED == 1
#include <iomanip>
#endif

Stopwatch::Stopwatch(bool displayOnDestruction, StopwatchDisplayMode displayMode)
    : displayOnDestruction(displayOnDestruction), displayMode(displayMode) {
  // Measures start time on initialization.
  Clock::getUptime(&startTime);
}

void Stopwatch::start() { Clock::getUptime(&startTime); }

dur_millis_t Stopwatch::stop(bool display) {
  stopInternal();
  if (display) {
    this->display();
  }
  return elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000;
}

double Stopwatch::stopSeconds() {
  stopInternal();
  return timevalOperations::toDouble(elapsedTime);
}

void Stopwatch::display() {
  if (displayMode == StopwatchDisplayMode::MILLIS) {
    dur_millis_t timeMillis =
        static_cast<dur_millis_t>(elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000);
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Stopwatch: Operation took " << timeMillis << " milliseconds" << std::endl;
#else
    sif::printInfo("Stopwatch: Operation took %lu milliseconds\n\r",
                   static_cast<unsigned int>(timeMillis));
#endif
  } else if (displayMode == StopwatchDisplayMode::SECONDS) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Stopwatch: Operation took " << std::setprecision(3) << std::fixed
              << timevalOperations::toDouble(elapsedTime) << " seconds" << std::endl;
#else
    sif::printInfo("Stopwatch: Operation took %.3f seconds\n\r",
                   static_cast<float>(timevalOperations::toDouble(elapsedTime)));
#endif
  }
}

Stopwatch::~Stopwatch() {
  if (displayOnDestruction) {
    stopInternal();
    display();
  }
}

void Stopwatch::setDisplayMode(StopwatchDisplayMode displayMode) {
  this->displayMode = displayMode;
}

StopwatchDisplayMode Stopwatch::getDisplayMode() const { return displayMode; }

void Stopwatch::stopInternal() {
  timeval endTime;
  Clock::getUptime(&endTime);
  elapsedTime = endTime - startTime;
}
