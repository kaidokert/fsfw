#ifndef CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_
#define CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_

#include <cstdint>
#include <fsfw/objectmanager/frameworkObjects.h>

// The objects will be instantiated in the ID order
namespace objects {
	enum sourceObjects: uint32_t {
		/* All addresses between start and end are reserved for the FSFW */
		FSFW_CONFIG_RESERVED_START = PUS_SERVICE_1_VERIFICATION,
		FSFW_CONFIG_RESERVED_END = TM_STORE
	};
}

#endif /* BSP_CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_ */
