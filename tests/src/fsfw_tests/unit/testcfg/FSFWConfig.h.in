#ifndef CONFIG_FSFWCONFIG_H_
#define CONFIG_FSFWCONFIG_H_

#include <cstddef>
#include <cstdint>

//! Used to determine whether C++ ostreams are used which can increase
//! the binary size significantly. If this is disabled,
//! the C stdio functions can be used alternatively
#define FSFW_CPP_OSTREAM_ENABLED 	    0

//! More FSFW related printouts depending on level. Useful for development.
#define FSFW_VERBOSE_LEVEL			    1

//! Can be used to completely disable printouts, even the C stdio ones.
#if FSFW_CPP_OSTREAM_ENABLED == 0 && FSFW_VERBOSE_LEVEL == 0
	#define FSFW_DISABLE_PRINTOUT 	    0
#endif

#define FSFW_USE_PUS_C_TELEMETRY        1
#define FSFW_USE_PUS_C_TELECOMMANDS     1

//! Can be used to disable the ANSI color sequences for C stdio.
#define FSFW_COLORED_OUTPUT 		    1

//! If FSFW_OBJ_EVENT_TRANSLATION is set to one,
//! additional output which requires the translation files translateObjects
//! and translateEvents (and their compiled source files)
#define FSFW_OBJ_EVENT_TRANSLATION	    0

#if FSFW_OBJ_EVENT_TRANSLATION == 1
//! Specify whether info events are printed too.
#define FSFW_DEBUG_INFO				    1
#include "objects/translateObjects.h"
#include "events/translateEvents.h"
#else
#endif

//! When using the newlib nano library, C99 support for stdio facilities
//! will not be provided. This define should be set to 1 if this is the case.
#define FSFW_NO_C99_IO 	                1

//! Specify whether a special mode store is used for Subsystem components.
#define FSFW_USE_MODESTORE              0

//! Defines if the real time scheduler for linux should be used.
//! If set to 0, this will also disable priority settings for linux
//! as most systems will not allow to set nice values without privileges
//! For embedded linux system set this to 1.
//! If set to 1 the binary needs "cap_sys_nice=eip" privileges to run
#define FSFW_USE_REALTIME_FOR_LINUX     1

namespace fsfwconfig {

//! Default timestamp size. The default timestamp will be an seven byte CDC short timestamp.
static constexpr uint8_t FSFW_MISSION_TIMESTAMP_SIZE = 7;

//! Configure the allocated pool sizes for the event manager.
static constexpr size_t FSFW_EVENTMGMR_MATCHTREE_NODES = 240;
static constexpr size_t FSFW_EVENTMGMT_EVENTIDMATCHERS = 120;
static constexpr size_t FSFW_EVENTMGMR_RANGEMATCHERS   = 120;

//! Defines the FIFO depth of each commanding service base which
//! also determines how many commands a CSB service can handle in one cycle
//! simultaneously. This will increase the required RAM for
//! each CSB service !
static constexpr uint8_t FSFW_CSB_FIFO_DEPTH = 6;

static constexpr size_t FSFW_PRINT_BUFFER_SIZE = 124;

static constexpr size_t FSFW_MAX_TM_PACKET_SIZE = 2048;

}

#endif /* CONFIG_FSFWCONFIG_H_ */
