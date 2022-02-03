#ifndef FSFW_TIMEMANAGER_STOPWATCH_H_
#define FSFW_TIMEMANAGER_STOPWATCH_H_

#include "Clock.h"

enum class StopwatchDisplayMode { MILLIS, SECONDS };

/**
 * @brief 	Simple Stopwatch implementation to measure elapsed time
 * @details
 * This class can be used to measure elapsed times. It also displays elapsed
 * times automatically on destruction if not explicitely deactivated in the
 * constructor. The default time format is the elapsed time in miliseconds
 * in seconds as a double.
 * @author 	R. Mueller
 */
class Stopwatch {
 public:
  /**
   * Default constructor. Call "Stopwatch stopwatch" without brackets if
   * no parameters are required!
   * @param displayOnDestruction If set to true, displays measured time on
   *        object destruction
   * @param displayMode Display format is either MS rounded or MS as double
   *        format
   * @param outputPrecision If using double format, specify precision here.
   */
  Stopwatch(bool displayOnDestruction = true,
            StopwatchDisplayMode displayMode = StopwatchDisplayMode::MILLIS);
  virtual ~Stopwatch();

  /**
   * Caches the start time
   */
  void start();

  /**
   * Calculates the elapsed time since start and returns it
   * @return elapsed time in milliseconds (rounded)
   */
  dur_millis_t stop(bool display = false);
  /**
   * Calculates the elapsed time since start and returns it
   * @return elapsed time in seconds (double precision)
   */
  double stopSeconds();

  /**
   * Displays the elapsed times on the osstream, depending on internal display
   * mode.
   */
  void display();

  StopwatchDisplayMode getDisplayMode() const;
  void setDisplayMode(StopwatchDisplayMode displayMode);
  bool displayOnDestruction = true;

 private:
  timeval startTime{0, 0};
  timeval elapsedTime{0, 0};

  StopwatchDisplayMode displayMode = StopwatchDisplayMode::MILLIS;

  void stopInternal();
};

#endif /* FSFW_TIMEMANAGER_STOPWATCH_H_ */
