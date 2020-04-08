/**
 * @file Stopwatch.h
 *
 * @date 08.04.2020
 */

#ifndef FRAMEWORK_TIMEMANAGER_STOPWATCH_H_
#define FRAMEWORK_TIMEMANAGER_STOPWATCH_H_
#include <framework/timemanager/Clock.h>

enum class StopwatchDisplayMode {
    MS_DOUBLE,
    MS
};

/**
 * @brief Simple Stopwatch implementation to measure elapsed time
 * @details
 * This class can be used to measure elapsed times. It also displays elapsed
 * times automatically on destruction if not explicitely deactivated in the
 * constructor. The default time format is the elapsed time in miliseconds
 * as a float.
 */
class Stopwatch {

    Stopwatch(bool displayOnDestruction = true,
            StopwatchDisplayMode displayMode = StopwatchDisplayMode::MS_DOUBLE);
    virtual~ Stopwatch();

    /**
     * Caches the start time
     */
    void start();

    /**
     * Calculates the elapsed time since start and returns it
     * @return elapsed time in milliseconds (rounded)
     */
    ms_normal_t stop();

    /**
     * Calculates the elapsed time since start and returns it
     * @return elapsed time in milliseconds (doulbe precision)
     */
    ms_double_t stopPrecise();

    /**
     * Displays the elapsed times on the osstream, depending on internal display
     * mode.
     */
    void display();

    StopwatchDisplayMode getDisplayMode() const;
    void setDisplayMode(StopwatchDisplayMode displayMode);
private:

    timeval startTime {0, 0};
    timeval elapsedTime {0, 0};
    double elapsedTimeMsDouble = 0;

    bool displayOnDestruction = true;
    StopwatchDisplayMode displayMode = StopwatchDisplayMode::MS_DOUBLE;

    void stopInternal();
};




#endif /* FRAMEWORK_TIMEMANAGER_STOPWATCH_H_ */
