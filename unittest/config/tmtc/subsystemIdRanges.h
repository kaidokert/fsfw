#ifndef CONFIG_TMTC_SUBSYSTEMIDRANGES_H_
#define CONFIG_TMTC_SUBSYSTEMIDRANGES_H_

#include <fsfw/events/fwSubsystemIdRanges.h>
#include <cstdint>

/**
 * These IDs are part of the ID for an event thrown by a subsystem.
 * Numbers 0-80 are reserved for FSFW Subsystem IDs (framework/events/)
 */
namespace SUBSYSTEM_ID {
enum: uint8_t {
	SUBSYSTE_ID_START = FW_SUBSYSTEM_ID_RANGE,
	/**
	 * 80-105: PUS Services
	 */
	PUS_SERVICE_2 = 82,
	PUS_SERVICE_3 = 83,
	PUS_SERVICE_5 = 85,
	PUS_SERVICE_6 = 86,
	PUS_SERVICE_8 = 88,
	PUS_SERVICE_23 = 91,
	DUMMY_DEVICE = 100,
	/**
	 * 105-115: AOCS
	 */
	GPS_DEVICE = 105,

	SPI_COM_IF = 128,
	I2C_COM_IF = 138
};
}

#endif /* CONFIG_TMTC_SUBSYSTEMIDRANGES_H_ */
