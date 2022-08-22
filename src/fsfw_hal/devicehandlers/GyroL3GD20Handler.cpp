#include "GyroL3GD20Handler.h"

#include <cmath>

#include "fsfw/datapool/PoolReadGuard.h"

GyroHandlerL3GD20H::GyroHandlerL3GD20H(object_id_t objectId, object_id_t deviceCommunication,
                                       CookieIF *comCookie, uint32_t transitionDelayMs)
    : DeviceHandlerBase(objectId, deviceCommunication, comCookie),
      transitionDelayMs(transitionDelayMs),
      dataset(this) {}

GyroHandlerL3GD20H::~GyroHandlerL3GD20H() {}

void GyroHandlerL3GD20H::doStartUp() {
  if (internalState == InternalState::NONE) {
    internalState = InternalState::CONFIGURE;
  }

  if (internalState == InternalState::CONFIGURE) {
    if (commandExecuted) {
      internalState = InternalState::CHECK_REGS;
      commandExecuted = false;
    }
  }

  if (internalState == InternalState::CHECK_REGS) {
    if (commandExecuted) {
      internalState = InternalState::NORMAL;
      if (goNormalModeImmediately) {
        setMode(MODE_NORMAL);
      } else {
        setMode(_MODE_TO_ON);
      }
      commandExecuted = false;
    }
  }
}

void GyroHandlerL3GD20H::doShutDown() { setMode(_MODE_POWER_DOWN); }

ReturnValue_t GyroHandlerL3GD20H::buildTransitionDeviceCommand(DeviceCommandId_t *id) {
  switch (internalState) {
    case (InternalState::NONE):
    case (InternalState::NORMAL): {
      return NOTHING_TO_SEND;
    }
    case (InternalState::CONFIGURE): {
      *id = L3GD20H::CONFIGURE_CTRL_REGS;
      uint8_t command[5];
      command[0] = L3GD20H::CTRL_REG_1_VAL;
      command[1] = L3GD20H::CTRL_REG_2_VAL;
      command[2] = L3GD20H::CTRL_REG_3_VAL;
      command[3] = L3GD20H::CTRL_REG_4_VAL;
      command[4] = L3GD20H::CTRL_REG_5_VAL;
      return buildCommandFromCommand(*id, command, 5);
    }
    case (InternalState::CHECK_REGS): {
      *id = L3GD20H::READ_REGS;
      return buildCommandFromCommand(*id, nullptr, 0);
    }
    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
      /* Might be a configuration error. */
      sif::warning << "GyroL3GD20Handler::buildTransitionDeviceCommand: "
                      "Unknown internal state!"
                   << std::endl;
#else
      sif::printDebug(
          "GyroL3GD20Handler::buildTransitionDeviceCommand: "
          "Unknown internal state!\n");
#endif
      return returnvalue::OK;
  }
  return returnvalue::OK;
}

ReturnValue_t GyroHandlerL3GD20H::buildNormalDeviceCommand(DeviceCommandId_t *id) {
  *id = L3GD20H::READ_REGS;
  return buildCommandFromCommand(*id, nullptr, 0);
}

ReturnValue_t GyroHandlerL3GD20H::buildCommandFromCommand(DeviceCommandId_t deviceCommand,
                                                          const uint8_t *commandData,
                                                          size_t commandDataLen) {
  switch (deviceCommand) {
    case (L3GD20H::READ_REGS): {
      commandBuffer[0] = L3GD20H::READ_START | L3GD20H::AUTO_INCREMENT_MASK | L3GD20H::READ_MASK;
      std::memset(commandBuffer + 1, 0, L3GD20H::READ_LEN);
      rawPacket = commandBuffer;
      rawPacketLen = L3GD20H::READ_LEN + 1;
      break;
    }
    case (L3GD20H::CONFIGURE_CTRL_REGS): {
      commandBuffer[0] = L3GD20H::CTRL_REG_1 | L3GD20H::AUTO_INCREMENT_MASK;
      if (commandData == nullptr or commandDataLen != 5) {
        return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
      }

      ctrlReg1Value = commandData[0];
      ctrlReg2Value = commandData[1];
      ctrlReg3Value = commandData[2];
      ctrlReg4Value = commandData[3];
      ctrlReg5Value = commandData[4];

      bool fsH = ctrlReg4Value & L3GD20H::SET_FS_1;
      bool fsL = ctrlReg4Value & L3GD20H::SET_FS_0;

      if (not fsH and not fsL) {
        sensitivity = L3GD20H::SENSITIVITY_00;
      } else if (not fsH and fsL) {
        sensitivity = L3GD20H::SENSITIVITY_01;
      } else {
        sensitivity = L3GD20H::SENSITIVITY_11;
      }

      commandBuffer[1] = ctrlReg1Value;
      commandBuffer[2] = ctrlReg2Value;
      commandBuffer[3] = ctrlReg3Value;
      commandBuffer[4] = ctrlReg4Value;
      commandBuffer[5] = ctrlReg5Value;

      rawPacket = commandBuffer;
      rawPacketLen = 6;
      break;
    }
    case (L3GD20H::READ_CTRL_REGS): {
      commandBuffer[0] = L3GD20H::READ_START | L3GD20H::AUTO_INCREMENT_MASK | L3GD20H::READ_MASK;

      std::memset(commandBuffer + 1, 0, 5);
      rawPacket = commandBuffer;
      rawPacketLen = 6;
      break;
    }
    default:
      return DeviceHandlerIF::COMMAND_NOT_IMPLEMENTED;
  }
  return returnvalue::OK;
}

ReturnValue_t GyroHandlerL3GD20H::scanForReply(const uint8_t *start, size_t len,
                                               DeviceCommandId_t *foundId, size_t *foundLen) {
  // For SPI, the ID will always be the one of the last sent command
  *foundId = this->getPendingCommand();
  *foundLen = this->rawPacketLen;

  return returnvalue::OK;
}

ReturnValue_t GyroHandlerL3GD20H::interpretDeviceReply(DeviceCommandId_t id,
                                                       const uint8_t *packet) {
  ReturnValue_t result = returnvalue::OK;
  switch (id) {
    case (L3GD20H::CONFIGURE_CTRL_REGS): {
      commandExecuted = true;
      break;
    }
    case (L3GD20H::READ_CTRL_REGS): {
      if (packet[1] == ctrlReg1Value and packet[2] == ctrlReg2Value and
          packet[3] == ctrlReg3Value and packet[4] == ctrlReg4Value and
          packet[5] == ctrlReg5Value) {
        commandExecuted = true;
      } else {
        // Attempt reconfiguration
        internalState = InternalState::CONFIGURE;
        return DeviceHandlerIF::DEVICE_REPLY_INVALID;
      }
      break;
    }
    case (L3GD20H::READ_REGS): {
      if (packet[1] != ctrlReg1Value and packet[2] != ctrlReg2Value and
          packet[3] != ctrlReg3Value and packet[4] != ctrlReg4Value and
          packet[5] != ctrlReg5Value) {
        return DeviceHandlerIF::DEVICE_REPLY_INVALID;
      } else {
        if (internalState == InternalState::CHECK_REGS) {
          commandExecuted = true;
        }
      }

      statusReg = packet[L3GD20H::STATUS_IDX];

      int16_t angVelocXRaw = packet[L3GD20H::OUT_X_H] << 8 | packet[L3GD20H::OUT_X_L];
      int16_t angVelocYRaw = packet[L3GD20H::OUT_Y_H] << 8 | packet[L3GD20H::OUT_Y_L];
      int16_t angVelocZRaw = packet[L3GD20H::OUT_Z_H] << 8 | packet[L3GD20H::OUT_Z_L];
      float angVelocX = angVelocXRaw * sensitivity;
      float angVelocY = angVelocYRaw * sensitivity;
      float angVelocZ = angVelocZRaw * sensitivity;

      int8_t temperaturOffset = (-1) * packet[L3GD20H::TEMPERATURE_IDX];
      float temperature = 25.0 + temperaturOffset;
      if (periodicPrintout) {
        if (debugDivider.checkAndIncrement()) {
          /* Set terminal to utf-8 if there is an issue with micro printout. */
#if FSFW_CPP_OSTREAM_ENABLED == 1
          sif::info << "GyroHandlerL3GD20H: Angular velocities (deg/s):" << std::endl;
          sif::info << "X: " << angVelocX << std::endl;
          sif::info << "Y: " << angVelocY << std::endl;
          sif::info << "Z: " << angVelocZ << std::endl;
#else
          sif::printInfo("GyroHandlerL3GD20H: Angular velocities (deg/s):\n");
          sif::printInfo("X: %f\n", angVelocX);
          sif::printInfo("Y: %f\n", angVelocY);
          sif::printInfo("Z: %f\n", angVelocZ);
#endif
        }
      }

      PoolReadGuard readSet(&dataset);
      if (readSet.getReadResult() == returnvalue::OK) {
        if (std::abs(angVelocX) < this->absLimitX) {
          dataset.angVelocX = angVelocX;
          dataset.angVelocX.setValid(true);
        } else {
          dataset.angVelocX.setValid(false);
        }

        if (std::abs(angVelocY) < this->absLimitY) {
          dataset.angVelocY = angVelocY;
          dataset.angVelocY.setValid(true);
        } else {
          dataset.angVelocY.setValid(false);
        }

        if (std::abs(angVelocZ) < this->absLimitZ) {
          dataset.angVelocZ = angVelocZ;
          dataset.angVelocZ.setValid(true);
        } else {
          dataset.angVelocZ.setValid(false);
        }

        dataset.temperature = temperature;
        dataset.temperature.setValid(true);
      }
      break;
    }
    default:
      return DeviceHandlerIF::COMMAND_NOT_IMPLEMENTED;
  }
  return result;
}

uint32_t GyroHandlerL3GD20H::getTransitionDelayMs(Mode_t from, Mode_t to) {
  return this->transitionDelayMs;
}

void GyroHandlerL3GD20H::setToGoToNormalMode(bool enable) { this->goNormalModeImmediately = true; }

ReturnValue_t GyroHandlerL3GD20H::initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
                                                          LocalDataPoolManager &poolManager) {
  localDataPoolMap.emplace(L3GD20H::ANG_VELOC_X, new PoolEntry<float>({0.0}));
  localDataPoolMap.emplace(L3GD20H::ANG_VELOC_Y, new PoolEntry<float>({0.0}));
  localDataPoolMap.emplace(L3GD20H::ANG_VELOC_Z, new PoolEntry<float>({0.0}));
  localDataPoolMap.emplace(L3GD20H::TEMPERATURE, new PoolEntry<float>({0.0}));
  return returnvalue::OK;
}

void GyroHandlerL3GD20H::fillCommandAndReplyMap() {
  insertInCommandAndReplyMap(L3GD20H::READ_REGS, 1, &dataset);
  insertInCommandAndReplyMap(L3GD20H::CONFIGURE_CTRL_REGS, 1);
  insertInCommandAndReplyMap(L3GD20H::READ_CTRL_REGS, 1);
}

void GyroHandlerL3GD20H::modeChanged() { internalState = InternalState::NONE; }

void GyroHandlerL3GD20H::setAbsoluteLimits(float limitX, float limitY, float limitZ) {
  this->absLimitX = limitX;
  this->absLimitY = limitY;
  this->absLimitZ = limitZ;
}

void GyroHandlerL3GD20H::enablePeriodicPrintouts(bool enable, uint8_t divider) {
  periodicPrintout = enable;
  debugDivider.setDivider(divider);
}
