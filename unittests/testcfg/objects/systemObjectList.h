#ifndef HOSTED_CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_
#define HOSTED_CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_

#include <cstdint>

#include "fsfw/objectmanager/frameworkObjects.h"

// The objects will be instantiated in the ID order
namespace objects {
enum sourceObjects : uint32_t {
  /* All addresses between start and end are reserved for the FSFW */
  FSFW_CONFIG_RESERVED_START = PUS_SERVICE_1_VERIFICATION,
  FSFW_CONFIG_RESERVED_END = TM_STORE,

  UDP_BRIDGE = 15,
  UDP_POLLING_TASK = 16,

  TEST_ECHO_COM_IF = 20,
  TEST_DEVICE = 21,

  HK_RECEIVER_MOCK = 22,
  TEST_LOCAL_POOL_OWNER_BASE = 25,

  SHARED_SET_ID = 26,

  DUMMY_POWER_SWITCHER = 28,

  DEVICE_HANDLER_MOCK = 29,
  COM_IF_MOCK = 30,
  DEVICE_HANDLER_COMMANDER = 40,
};
}

#endif /* HOSTED_CONFIG_OBJECTS_SYSTEMOBJECTLIST_H_ */
