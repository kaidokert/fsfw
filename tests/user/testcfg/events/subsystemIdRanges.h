#ifndef CONFIG_EVENTS_SUBSYSTEMIDRANGES_H_
#define CONFIG_EVENTS_SUBSYSTEMIDRANGES_H_

#include <cstdint>
#include <fsfw/events/fwSubsystemIdRanges.h>

/**
 * @brief	Custom subsystem IDs can be added here
 * @details
 * Subsystem IDs are used to create unique events.
 */
namespace SUBSYSTEM_ID {
enum: uint8_t {
	SUBSYSTEM_ID_START = FW_SUBSYSTEM_ID_RANGE,
};
}

#endif /* CONFIG_EVENTS_SUBSYSTEMIDRANGES_H_ */
