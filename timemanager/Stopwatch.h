/**
 * @file Stopwatch.h
 *
 * @date 08.04.2020
 */

#ifndef FRAMEWORK_TIMEMANAGER_STOPWATCH_H_
#define FRAMEWORK_TIMEMANAGER_STOPWATCH_H_
#include <framework/timemanager/Clock.h>

class Stopwatch {
    enum class DisplayMode {
        MS_FLOAT,
        MS
    };

    Stopwatch(bool displayOnDestruction = true, DisplayMode displayMode =
            DisplayMode::MS_FLOAT);
    virtual~ Stopwatch();

    void start();

    ms_normal_t stop();
    ms_double_t stopPrecise();

    void display();
private:
    static const ms_normal_t INVALID_TIME = 0xFFFFFFFF;

    timeval startTime {0, 0};
    timeval stopTime {0, 0};
    timeval elapsedTime {0, 0};
    double elapsedTimeMs = 0;
    //ms_normal_t elapsedTimeMs {0};

    bool displayOnDestruction = true;

    enum class StopwatchState {
        IDLE,
        STARTED,
    };

    StopwatchState stopwatchState = StopwatchState::IDLE;
    DisplayMode displayMode = DisplayMode::MS_FLOAT;

    void stopInternal();
};




#endif /* FRAMEWORK_TIMEMANAGER_STOPWATCH_H_ */
