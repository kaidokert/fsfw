#ifndef MISSION_CONTROLLER_CTRLDEFINITIONS_TESTCTRLDEFINITIONS_H_
#define MISSION_CONTROLLER_CTRLDEFINITIONS_TESTCTRLDEFINITIONS_H_

#include <OBSWConfig.h>
#include <fsfw/objectmanager/SystemObjectIF.h>

namespace testcontroller {

enum sourceObjectIds : object_id_t {
  DEVICE_0_ID = objects::TEST_DEVICE_HANDLER_0,
  DEVICE_1_ID = objects::TEST_DEVICE_HANDLER_1,
};

}

#endif /* MISSION_CONTROLLER_CTRLDEFINITIONS_TESTCTRLDEFINITIONS_H_ */
