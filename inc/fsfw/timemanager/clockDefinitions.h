#ifndef FSFW_TIMEMANAGER_CLOCKDEFINITIONS_H_
#define FSFW_TIMEMANAGER_CLOCKDEFINITIONS_H_

#include <cstdint>

// I'd also like to include the TimeOfDay_t struct here, but that would
// break code which uses Clock::TimeOfDay_t. Solution would be to use
// a Clock namespace instead of class with static functions.

//! Don't use these for time points, type is not large enough for UNIX epoch.
using dur_millis_t = uint32_t;

#endif /* FSFW_TIMEMANAGER_CLOCKDEFINITIONS_H_ */
