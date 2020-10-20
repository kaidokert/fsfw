#ifndef CONFIG_FSFWCONFIG_H_
#define CONFIG_FSFWCONFIG_H_

#include <fsfw/unittest/config/version.h>

//! Used to determine whether C++ ostreams are used
//! Those can lead to code bloat.
#define FSFW_CPP_OSTREAM_ENABLED 	1

//! Reduced printout to further decrese code size
//! Be careful, this also turns off most diagnostic prinouts!
#define FSFW_REDUCED_PRINTOUT		0

//! If -DDEBUG is supplied in the build defines, there will be
//! additional output which requires the translation files translateObjects
//! and translateEvents (and their compiles source files)
#ifdef DEBUG
#define FSFW_DEBUG_OUTPUT 			1
//! Specify whether info events are printed too.
#define FSFW_DEBUG_INFO				1
#include <translateObjects.h>
#include <translateEvents.h>
#else
#define FSFW_DEBUG_OUTPUT			0
#endif

//! When using the newlib nano library, C99 support for stdio facilities
//! will not be provided. This define should be set to 1 if this is the case.
#define FSFW_NEWLIB_NANO_NO_C99_IO 	1


#endif /* CONFIG_FSFWCONFIG_H_ */
