#ifndef FSFW_TIMEMANAGER_CLOCKDEFINITIONS_H_
#define FSFW_TIMEMANAGER_CLOCKDEFINITIONS_H_

#include <cstdint>


typedef struct {
    uint32_t year; //!< Year, A.D.
    uint32_t month; //!< Month, 1 .. 12.
    uint32_t day; //!< Day, 1 .. 31.
    uint32_t hour; //!< Hour, 0 .. 23.
    uint32_t minute; //!< Minute, 0 .. 59.
    uint32_t second; //!< Second, 0 .. 59.
    uint32_t usecond; //!< Microseconds, 0 .. 999999
} TimeOfDay_t;

//! Don't use these for time points, type is not large enough for UNIX epoch.
using dur_millis_t = uint32_t;

#endif /* FSFW_TIMEMANAGER_CLOCKDEFINITIONS_H_ */
