#ifndef HOSTED_CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_
#define HOSTED_CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_

#include <cstdint>
#include <fsfw/objectmanager/frameworkObjects.h>

// The objects will be instantiated in the ID order
namespace objects {
	enum sourceObjects: uint32_t {
		/* All addresses between start and end are reserved for the FSFW */
		FSFW_CONFIG_RESERVED_START = PUS_SERVICE_1_VERIFICATION,
		FSFW_CONFIG_RESERVED_END = TM_STORE,

		CCSDS_DISTRIBUTOR = 10,
		PUS_DISTRIBUTOR = 11,
		TM_FUNNEL = 12,

		TCPIP_BRIDGE = 15,
		TCPIP_HELPER = 16,

		TEST_ECHO_COM_IF = 20,
		TEST_DEVICE = 21,

		HK_RECEIVER_MOCK = 22,
		TEST_LOCAL_POOL_OWNER_BASE = 25
	};
}

#endif /* BSP_CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_ */
