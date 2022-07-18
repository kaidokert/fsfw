#ifndef MISSION_DEVICES_DEVICEDEFINITIONS_GYROL3GD20DEFINITIONS_H_
#define MISSION_DEVICES_DEVICEDEFINITIONS_GYROL3GD20DEFINITIONS_H_

#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/devicehandlers/DeviceHandlerIF.h>

#include <cstdint>

namespace L3GD20H {

/* Actual size is 15 but we round up a bit */
static constexpr size_t MAX_BUFFER_SIZE = 16;

static constexpr uint8_t READ_MASK = 0b10000000;

static constexpr uint8_t AUTO_INCREMENT_MASK = 0b01000000;

static constexpr uint8_t WHO_AM_I_REG = 0b00001111;
static constexpr uint8_t WHO_AM_I_VAL = 0b11010111;

/*------------------------------------------------------------------------*/
/* Control registers */
/*------------------------------------------------------------------------*/
static constexpr uint8_t CTRL_REG_1 = 0b00100000;
static constexpr uint8_t CTRL_REG_2 = 0b00100001;
static constexpr uint8_t CTRL_REG_3 = 0b00100010;
static constexpr uint8_t CTRL_REG_4 = 0b00100011;
static constexpr uint8_t CTRL_REG_5 = 0b00100100;

/* Register 1 */
static constexpr uint8_t SET_DR_1 = 1 << 7;
static constexpr uint8_t SET_DR_0 = 1 << 6;
static constexpr uint8_t SET_BW_1 = 1 << 5;
static constexpr uint8_t SET_BW_0 = 1 << 4;
static constexpr uint8_t SET_POWER_NORMAL_MODE = 1 << 3;
static constexpr uint8_t SET_Z_ENABLE = 1 << 2;
static constexpr uint8_t SET_X_ENABLE = 1 << 1;
static constexpr uint8_t SET_Y_ENABLE = 1;

static constexpr uint8_t CTRL_REG_1_VAL =
    SET_POWER_NORMAL_MODE | SET_Z_ENABLE | SET_Y_ENABLE | SET_X_ENABLE;

/* Register 2 */
static constexpr uint8_t EXTERNAL_EDGE_ENB = 1 << 7;
static constexpr uint8_t LEVEL_SENSITIVE_TRIGGER = 1 << 6;
static constexpr uint8_t SET_HPM_1 = 1 << 5;
static constexpr uint8_t SET_HPM_0 = 1 << 4;
static constexpr uint8_t SET_HPCF_3 = 1 << 3;
static constexpr uint8_t SET_HPCF_2 = 1 << 2;
static constexpr uint8_t SET_HPCF_1 = 1 << 1;
static constexpr uint8_t SET_HPCF_0 = 1;

static constexpr uint8_t CTRL_REG_2_VAL = 0b00000000;

/* Register 3 */
static constexpr uint8_t CTRL_REG_3_VAL = 0b00000000;

/* Register 4 */
static constexpr uint8_t SET_BNU = 1 << 7;
static constexpr uint8_t SET_BLE = 1 << 6;
static constexpr uint8_t SET_FS_1 = 1 << 5;
static constexpr uint8_t SET_FS_0 = 1 << 4;
static constexpr uint8_t SET_IMP_ENB = 1 << 3;
static constexpr uint8_t SET_SELF_TEST_ENB_1 = 1 << 2;
static constexpr uint8_t SET_SELF_TEST_ENB_0 = 1 << 1;
static constexpr uint8_t SET_SPI_IF_SELECT = 1;

/* Enable big endian data format */
static constexpr uint8_t CTRL_REG_4_VAL = SET_BLE;

/* Register 5 */
static constexpr uint8_t SET_REBOOT_MEM = 1 << 7;
static constexpr uint8_t SET_FIFO_ENB = 1 << 6;

static constexpr uint8_t CTRL_REG_5_VAL = 0b00000000;

/* Possible range values in degrees per second (DPS). */
static constexpr uint16_t RANGE_DPS_00 = 245;
static constexpr float SENSITIVITY_00 = 8.75 * 0.001;
static constexpr uint16_t RANGE_DPS_01 = 500;
static constexpr float SENSITIVITY_01 = 17.5 * 0.001;
static constexpr uint16_t RANGE_DPS_11 = 2000;
static constexpr float SENSITIVITY_11 = 70.0 * 0.001;

static constexpr uint8_t READ_START = CTRL_REG_1;
static constexpr size_t READ_LEN = 14;

/* Indexing */
static constexpr uint8_t REFERENCE_IDX = 6;
static constexpr uint8_t TEMPERATURE_IDX = 7;
static constexpr uint8_t STATUS_IDX = 8;
static constexpr uint8_t OUT_X_H = 9;
static constexpr uint8_t OUT_X_L = 10;
static constexpr uint8_t OUT_Y_H = 11;
static constexpr uint8_t OUT_Y_L = 12;
static constexpr uint8_t OUT_Z_H = 13;
static constexpr uint8_t OUT_Z_L = 14;

/*------------------------------------------------------------------------*/
/* Device Handler specific */
/*------------------------------------------------------------------------*/
static constexpr DeviceCommandId_t READ_REGS = 0;
static constexpr DeviceCommandId_t CONFIGURE_CTRL_REGS = 1;
static constexpr DeviceCommandId_t READ_CTRL_REGS = 2;

static constexpr uint32_t GYRO_DATASET_ID = READ_REGS;

enum GyroPoolIds : lp_id_t { ANG_VELOC_X, ANG_VELOC_Y, ANG_VELOC_Z, TEMPERATURE };

}  // namespace L3GD20H

class GyroPrimaryDataset : public StaticLocalDataSet<5> {
 public:
  /** Constructor  for data users like controllers */
  GyroPrimaryDataset(object_id_t mgmId)
      : StaticLocalDataSet(sid_t(mgmId, L3GD20H::GYRO_DATASET_ID)) {
    setAllVariablesReadOnly();
  }

  /* Angular velocities in degrees per second (DPS) */
  lp_var_t<float> angVelocX = lp_var_t<float>(sid.objectId, L3GD20H::ANG_VELOC_X, this);
  lp_var_t<float> angVelocY = lp_var_t<float>(sid.objectId, L3GD20H::ANG_VELOC_Y, this);
  lp_var_t<float> angVelocZ = lp_var_t<float>(sid.objectId, L3GD20H::ANG_VELOC_Z, this);
  lp_var_t<float> temperature = lp_var_t<float>(sid.objectId, L3GD20H::TEMPERATURE, this);

 private:
  friend class GyroHandlerL3GD20H;
  /** Constructor  for the data creator */
  GyroPrimaryDataset(HasLocalDataPoolIF* hkOwner)
      : StaticLocalDataSet(hkOwner, L3GD20H::GYRO_DATASET_ID) {}
};

#endif /* MISSION_DEVICES_DEVICEDEFINITIONS_GYROL3GD20DEFINITIONS_H_ */
