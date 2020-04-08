/**
 * @file Stopwatch.h
 *
 * @date 08.04.2020
 */

#ifndef FRAMEWORK_TIMEMANAGER_STOPWATCH_H_
#define FRAMEWORK_TIMEMANAGER_STOPWATCH_H_
#include <framework/timemanager/Clock.h>

/**
 * @brief Simple Stopwatch implementation to measure elapsed time
 * @details
 * This class can be used to measure elapsed times. It also displays elapsed
 * times automatically on destruction if not explicitely deactivated in the
 * constructor. The default time format is the elapsed time in miliseconds
 * as a float.
 * This class caches the value
 */
class Stopwatch {
    enum class DisplayMode {
        MS_DOUBLE,
        MS
    };

    Stopwatch(bool displayOnDestruction = true, DisplayMode displayMode =
            DisplayMode::MS_DOUBLE);
    virtual~ Stopwatch();

    void start();

    ms_normal_t stop();
    ms_double_t stopPrecise();

    void display();
private:

    timeval startTime {0, 0};
    timeval elapsedTime {0, 0};
    double elapsedTimeMsDouble = 0;

    bool displayOnDestruction = true;
    DisplayMode displayMode = DisplayMode::MS_DOUBLE;

    void stopInternal();
};




#endif /* FRAMEWORK_TIMEMANAGER_STOPWATCH_H_ */
