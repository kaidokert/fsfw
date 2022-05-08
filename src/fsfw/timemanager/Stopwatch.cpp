#include "fsfw/timemanager/Stopwatch.h"

#include "fsfw/serviceinterface.h"

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
    auto timeMillis =
        static_cast<dur_millis_t>(elapsedTime.tv_sec * 1000 + elapsedTime.tv_usec / 1000);
    FSFW_LOGIT("Stopwatch::display: {} ms elapsed\n", timeMillis);
  } else if (displayMode == StopwatchDisplayMode::SECONDS) {
    FSFW_LOGIT("Stopwatch::display: {} seconds elapsed\n",
               static_cast<float>(timevalOperations::toDouble(elapsedTime)));
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
