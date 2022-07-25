#ifndef MISSION_DEVICES_GYROL3GD20HANDLER_H_
#define MISSION_DEVICES_GYROL3GD20HANDLER_H_

#include <fsfw/devicehandlers/DeviceHandlerBase.h>
#include <fsfw/globalfunctions/PeriodicOperationDivider.h>

#include "devicedefinitions/GyroL3GD20Definitions.h"

/**
 * @brief 	Device Handler for the L3GD20H gyroscope sensor
 *          (https://www.st.com/en/mems-and-sensors/l3gd20h.html)
 * @details
 * Advanced documentation:
 * https://egit.irs.uni-stuttgart.de/redmine/projects/eive-flight-manual/wiki/L3GD20H_Gyro
 *
 * Data is read big endian with the smallest possible range of 245 degrees per second.
 */
class GyroHandlerL3GD20H : public DeviceHandlerBase {
 public:
  GyroHandlerL3GD20H(object_id_t objectId, object_id_t deviceCommunication, CookieIF *comCookie,
                     uint32_t transitionDelayMs);
  virtual ~GyroHandlerL3GD20H();

  void enablePeriodicPrintouts(bool enable, uint8_t divider);

  /**
   * Set the absolute limit for the values on the axis in degrees per second.
   * The dataset values will be marked as invalid if that limit is exceeded
   * @param xLimit
   * @param yLimit
   * @param zLimit
   */
  void setAbsoluteLimits(float limitX, float limitY, float limitZ);

  /**
   * @brief   Configure device handler to go to normal mode immediately
   */
  void setToGoToNormalMode(bool enable);

 protected:
  /* DeviceHandlerBase overrides */
  ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t *id) override;
  void doStartUp() override;
  void doShutDown() override;
  ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t *id) override;
  ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand, const uint8_t *commandData,
                                        size_t commandDataLen) override;
  ReturnValue_t scanForReply(const uint8_t *start, size_t len, DeviceCommandId_t *foundId,
                             size_t *foundLen) override;
  virtual ReturnValue_t interpretDeviceReply(DeviceCommandId_t id, const uint8_t *packet) override;

  void fillCommandAndReplyMap() override;
  void modeChanged() override;
  virtual uint32_t getTransitionDelayMs(Mode_t from, Mode_t to) override;
  ReturnValue_t initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
                                        LocalDataPoolManager &poolManager) override;

 private:
  uint32_t transitionDelayMs = 0;
  GyroPrimaryDataset dataset;

  float absLimitX = L3GD20H::RANGE_DPS_00;
  float absLimitY = L3GD20H::RANGE_DPS_00;
  float absLimitZ = L3GD20H::RANGE_DPS_00;

  enum class InternalState { NONE, CONFIGURE, CHECK_REGS, NORMAL };
  InternalState internalState = InternalState::NONE;
  bool commandExecuted = false;

  uint8_t statusReg = 0;
  bool goNormalModeImmediately = false;

  uint8_t ctrlReg1Value = L3GD20H::CTRL_REG_1_VAL;
  uint8_t ctrlReg2Value = L3GD20H::CTRL_REG_2_VAL;
  uint8_t ctrlReg3Value = L3GD20H::CTRL_REG_3_VAL;
  uint8_t ctrlReg4Value = L3GD20H::CTRL_REG_4_VAL;
  uint8_t ctrlReg5Value = L3GD20H::CTRL_REG_5_VAL;

  uint8_t commandBuffer[L3GD20H::READ_LEN + 1];

  // Set default value
  float sensitivity = L3GD20H::SENSITIVITY_00;

  bool periodicPrintout = false;
  PeriodicOperationDivider debugDivider = PeriodicOperationDivider(3);
};

#endif /* MISSION_DEVICES_GYROL3GD20HANDLER_H_ */
