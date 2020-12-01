#ifndef CONFIG_FSFWCONFIG_H_
#define CONFIG_FSFWCONFIG_H_

#include <FSFWVersion.h>
#include <cstddef>
#include <cstdint>

//! Used to determine whether C++ ostreams are used
//! Those can lead to code bloat.
#define FSFW_CPP_OSTREAM_ENABLED 	1

//! Reduced printout to further decrese code size
//! Be careful, this also turns off most diagnostic prinouts!
#define FSFW_REDUCED_PRINTOUT		0

//! Can be used to enable debugging printouts for developing the FSFW
#define FSFW_DEBUGGING              0

//! Defines the FIFO depth of each commanding service base which
//! also determines how many commands a CSB service can handle in one cycle
//! simulataneously. This will increase the required RAM for
//! each CSB service !
#define FSFW_CSB_FIFO_DEPTH			6

//! If FSFW_OBJ_EVENT_TRANSLATION is set to one,
//! additional output which requires the translation files translateObjects
//! and translateEvents (and their compiled source files)
#define FSFW_OBJ_EVENT_TRANSLATION	0

#if FSFW_OBJ_EVENT_TRANSLATION == 1
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
#define FSFW_NO_C99_IO 	1

namespace fsfwconfig {
//! Default timestamp size. The default timestamp will be an eight byte CDC
//! short timestamp.
static constexpr uint8_t FSFW_MISSION_TIMESTAMP_SIZE = 8;

//! Configure the allocated pool sizes for the event manager.
static constexpr size_t FSFW_EVENTMGMR_MATCHTREE_NODES = 240;
static constexpr size_t FSFW_EVENTMGMT_EVENTIDMATCHERS = 120;
static constexpr size_t FSFW_EVENTMGMR_RANGEMATCHERS   = 120;
}

#endif /* CONFIG_FSFWCONFIG_H_ */
