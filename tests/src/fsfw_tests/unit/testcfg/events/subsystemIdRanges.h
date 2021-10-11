#ifndef CONFIG_EVENTS_SUBSYSTEMIDRANGES_H_
#define CONFIG_EVENTS_SUBSYSTEMIDRANGES_H_

#include "fsfw/events/fwSubsystemIdRanges.h"
#include <cstdint>


/**
 * @brief	Custom subsystem IDs can be added here
 * @details
 * Subsystem IDs are used to create unique events.
 */
namespace SUBSYSTEM_ID {
enum: uint8_t {
	SUBSYSTEM_ID_START =  0,
	SUBSYSTEM_ID_END // [EXPORT] : [END]
};
}

#endif /* CONFIG_EVENTS_SUBSYSTEMIDRANGES_H_ */
