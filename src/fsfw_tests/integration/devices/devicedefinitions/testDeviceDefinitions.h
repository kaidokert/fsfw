#ifndef MISSION_DEVICES_DEVICEDEFINITIONS_TESTDEVICEDEFINITIONS_H_
#define MISSION_DEVICES_DEVICEDEFINITIONS_TESTDEVICEDEFINITIONS_H_

#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/devicehandlers/DeviceHandlerIF.h>

namespace testdevice {

enum ParameterUniqueIds : uint8_t {
  TEST_UINT32_0,
  TEST_INT32_1,
  TEST_FLOAT_VEC3_2,
  PERIODIC_PRINT_ENABLED,
  CHANGING_DATASETS
};

enum DeviceIndex : uint32_t { DEVICE_0, DEVICE_1 };

/** Normal mode command. This ID is also used to access the set variable via the housekeeping
service */
static constexpr DeviceCommandId_t TEST_NORMAL_MODE_CMD = 0;

//! Test completion reply
static constexpr DeviceCommandId_t TEST_COMMAND_0 = 1;
//! Test data reply
static constexpr DeviceCommandId_t TEST_COMMAND_1 = 2;

/**
 * Can be used to trigger a notification to the demo controller. For DEVICE_0, only notifications
 * messages will be generated while for DEVICE_1, snapshot messages will be generated.
 *
 * DEVICE_0 VAR:    Sets the set variable 0 above a treshold (200) to trigger a variable
 *                  notification.
 * DEVICE_0 SET:    Sets the vector mean values above a treshold (mean larger than 20) to trigger a
 *                  set notification.
 *
 * DEVICE_1 VAR:    Sets the set variable 0 below a treshold (less than 50 but not 0) to trigger a
 *                  variable snapshot.
 * DEVICE_1 SET:    Sets the set vector mean values below a treshold (mean smaller than -20) to
 *                  trigger a set snapshot message.
 */
static constexpr DeviceCommandId_t TEST_NOTIF_SNAPSHOT_VAR = 3;
static constexpr DeviceCommandId_t TEST_NOTIF_SNAPSHOT_SET = 4;

/**
 * Can be used to trigger a snapshot message to the demo controller.
 * Depending on the device index, a notification will be triggered for different set variables.
 *
 * DEVICE_0:    Sets the set variable 0 below a treshold (below 50 but not 0) to trigger
 *              a variable snapshot
 * DEVICE_1:    Sets the vector mean values below a treshold (mean less than -20) to trigger a
 *              set snapshot
 */
static constexpr DeviceCommandId_t TEST_SNAPSHOT = 5;

//! Generates a random value for variable 1 of the dataset.
static constexpr DeviceCommandId_t GENERATE_SET_VAR_1_RNG_VALUE = 6;

/**
 * These parameters are sent back with the command ID as a data reply
 */
static constexpr uint16_t COMMAND_1_PARAM1 = 0xBAB0;  //!< param1, 2 bytes
//! param2, 8 bytes
static constexpr uint64_t COMMAND_1_PARAM2 = 0x000000524F42494E;

static constexpr size_t TEST_COMMAND_0_SIZE = sizeof(TEST_COMMAND_0);
static constexpr size_t TEST_COMMAND_1_SIZE =
    sizeof(TEST_COMMAND_1) + sizeof(COMMAND_1_PARAM1) + sizeof(COMMAND_1_PARAM2);

enum PoolIds : lp_id_t { TEST_UINT8_ID = 0, TEST_UINT32_ID = 1, TEST_FLOAT_VEC_3_ID = 2 };

static constexpr uint8_t TEST_SET_ID = TEST_NORMAL_MODE_CMD;

class TestDataSet : public StaticLocalDataSet<3> {
 public:
  TestDataSet(HasLocalDataPoolIF* owner) : StaticLocalDataSet(owner, TEST_SET_ID) {}
  TestDataSet(object_id_t owner) : StaticLocalDataSet(sid_t(owner, TEST_SET_ID)) {}

  lp_var_t<uint8_t> testUint8Var =
      lp_var_t<uint8_t>(gp_id_t(this->getCreatorObjectId(), PoolIds::TEST_UINT8_ID), this);
  lp_var_t<uint32_t> testUint32Var =
      lp_var_t<uint32_t>(gp_id_t(this->getCreatorObjectId(), PoolIds::TEST_UINT32_ID), this);
  lp_vec_t<float, 3> testFloat3Vec =
      lp_vec_t<float, 3>(gp_id_t(this->getCreatorObjectId(), PoolIds::TEST_FLOAT_VEC_3_ID), this);
};

}  // namespace testdevice

#endif /* MISSION_DEVICES_DEVICEDEFINITIONS_TESTDEVICEDEFINITIONS_H_ */
