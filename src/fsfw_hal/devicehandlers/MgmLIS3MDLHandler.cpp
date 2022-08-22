#include "MgmLIS3MDLHandler.h"

#include <cmath>

#include "fsfw/datapool/PoolReadGuard.h"

MgmLIS3MDLHandler::MgmLIS3MDLHandler(object_id_t objectId, object_id_t deviceCommunication,
                                     CookieIF *comCookie, uint32_t transitionDelay)
    : DeviceHandlerBase(objectId, deviceCommunication, comCookie),
      dataset(this),
      transitionDelay(transitionDelay) {
  // Set to default values right away
  registers[0] = MGMLIS3MDL::CTRL_REG1_DEFAULT;
  registers[1] = MGMLIS3MDL::CTRL_REG2_DEFAULT;
  registers[2] = MGMLIS3MDL::CTRL_REG3_DEFAULT;
  registers[3] = MGMLIS3MDL::CTRL_REG4_DEFAULT;
  registers[4] = MGMLIS3MDL::CTRL_REG5_DEFAULT;
}

MgmLIS3MDLHandler::~MgmLIS3MDLHandler() {}

void MgmLIS3MDLHandler::doStartUp() {
  switch (internalState) {
    case (InternalState::STATE_NONE): {
      internalState = InternalState::STATE_FIRST_CONTACT;
      break;
    }
    case (InternalState::STATE_FIRST_CONTACT): {
      /* Will be set by checking device ID (WHO AM I register) */
      if (commandExecuted) {
        commandExecuted = false;
        internalState = InternalState::STATE_SETUP;
      }
      break;
    }
    case (InternalState::STATE_SETUP): {
      internalState = InternalState::STATE_CHECK_REGISTERS;
      break;
    }
    case (InternalState::STATE_CHECK_REGISTERS): {
      /* Set up cached registers which will be used to configure the MGM. */
      if (commandExecuted) {
        commandExecuted = false;
        if (goToNormalMode) {
          setMode(MODE_NORMAL);
        } else {
          setMode(_MODE_TO_ON);
        }
      }
      break;
    }
    default:
      break;
  }
}

void MgmLIS3MDLHandler::doShutDown() { setMode(_MODE_POWER_DOWN); }

ReturnValue_t MgmLIS3MDLHandler::buildTransitionDeviceCommand(DeviceCommandId_t *id) {
  switch (internalState) {
    case (InternalState::STATE_NONE):
    case (InternalState::STATE_NORMAL): {
      return DeviceHandlerBase::NOTHING_TO_SEND;
    }
    case (InternalState::STATE_FIRST_CONTACT): {
      *id = MGMLIS3MDL::IDENTIFY_DEVICE;
      break;
    }
    case (InternalState::STATE_SETUP): {
      *id = MGMLIS3MDL::SETUP_MGM;
      break;
    }
    case (InternalState::STATE_CHECK_REGISTERS): {
      *id = MGMLIS3MDL::READ_CONFIG_AND_DATA;
      break;
    }
    default: {
      /* might be a configuration error. */
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "GyroHandler::buildTransitionDeviceCommand: Unknown internal state!"
                   << std::endl;
#else
      sif::printWarning("GyroHandler::buildTransitionDeviceCommand: Unknown internal state!\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
      return returnvalue::OK;
    }
  }
  return buildCommandFromCommand(*id, NULL, 0);
}

uint8_t MgmLIS3MDLHandler::readCommand(uint8_t command, bool continuousCom) {
  command |= (1 << MGMLIS3MDL::RW_BIT);
  if (continuousCom == true) {
    command |= (1 << MGMLIS3MDL::MS_BIT);
  }
  return command;
}

uint8_t MgmLIS3MDLHandler::writeCommand(uint8_t command, bool continuousCom) {
  command &= ~(1 << MGMLIS3MDL::RW_BIT);
  if (continuousCom == true) {
    command |= (1 << MGMLIS3MDL::MS_BIT);
  }
  return command;
}

void MgmLIS3MDLHandler::setupMgm() {
  registers[0] = MGMLIS3MDL::CTRL_REG1_DEFAULT;
  registers[1] = MGMLIS3MDL::CTRL_REG2_DEFAULT;
  registers[2] = MGMLIS3MDL::CTRL_REG3_DEFAULT;
  registers[3] = MGMLIS3MDL::CTRL_REG4_DEFAULT;
  registers[4] = MGMLIS3MDL::CTRL_REG5_DEFAULT;

  prepareCtrlRegisterWrite();
}

ReturnValue_t MgmLIS3MDLHandler::buildNormalDeviceCommand(DeviceCommandId_t *id) {
  // Data/config register will be read in an alternating manner.
  if (communicationStep == CommunicationStep::DATA) {
    *id = MGMLIS3MDL::READ_CONFIG_AND_DATA;
    communicationStep = CommunicationStep::TEMPERATURE;
    return buildCommandFromCommand(*id, NULL, 0);
  } else {
    *id = MGMLIS3MDL::READ_TEMPERATURE;
    communicationStep = CommunicationStep::DATA;
    return buildCommandFromCommand(*id, NULL, 0);
  }
}

ReturnValue_t MgmLIS3MDLHandler::buildCommandFromCommand(DeviceCommandId_t deviceCommand,
                                                         const uint8_t *commandData,
                                                         size_t commandDataLen) {
  switch (deviceCommand) {
    case (MGMLIS3MDL::READ_CONFIG_AND_DATA): {
      std::memset(commandBuffer, 0, sizeof(commandBuffer));
      commandBuffer[0] = readCommand(MGMLIS3MDL::CTRL_REG1, true);

      rawPacket = commandBuffer;
      rawPacketLen = MGMLIS3MDL::NR_OF_DATA_AND_CFG_REGISTERS + 1;
      return returnvalue::OK;
    }
    case (MGMLIS3MDL::READ_TEMPERATURE): {
      std::memset(commandBuffer, 0, 3);
      commandBuffer[0] = readCommand(MGMLIS3MDL::TEMP_LOWBYTE, true);

      rawPacket = commandBuffer;
      rawPacketLen = 3;
      return returnvalue::OK;
    }
    case (MGMLIS3MDL::IDENTIFY_DEVICE): {
      return identifyDevice();
    }
    case (MGMLIS3MDL::TEMP_SENSOR_ENABLE): {
      return enableTemperatureSensor(commandData, commandDataLen);
    }
    case (MGMLIS3MDL::SETUP_MGM): {
      setupMgm();
      return returnvalue::OK;
    }
    case (MGMLIS3MDL::ACCURACY_OP_MODE_SET): {
      return setOperatingMode(commandData, commandDataLen);
    }
    default:
      return DeviceHandlerIF::COMMAND_NOT_IMPLEMENTED;
  }
  return returnvalue::FAILED;
}

ReturnValue_t MgmLIS3MDLHandler::identifyDevice() {
  uint32_t size = 2;
  commandBuffer[0] = readCommand(MGMLIS3MDL::IDENTIFY_DEVICE_REG_ADDR);
  commandBuffer[1] = 0x00;

  rawPacket = commandBuffer;
  rawPacketLen = size;

  return returnvalue::OK;
}

ReturnValue_t MgmLIS3MDLHandler::scanForReply(const uint8_t *start, size_t len,
                                              DeviceCommandId_t *foundId, size_t *foundLen) {
  *foundLen = len;
  if (len == MGMLIS3MDL::NR_OF_DATA_AND_CFG_REGISTERS + 1) {
    *foundLen = len;
    *foundId = MGMLIS3MDL::READ_CONFIG_AND_DATA;
    // Check validity by checking config registers
    if (start[1] != registers[0] or start[2] != registers[1] or start[3] != registers[2] or
        start[4] != registers[3] or start[5] != registers[4]) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "MGMHandlerLIS3MDL::scanForReply: Invalid registers!" << std::endl;
#else
      sif::printWarning("MGMHandlerLIS3MDL::scanForReply: Invalid registers!\n");
#endif
#endif
      return DeviceHandlerIF::INVALID_DATA;
    }
    if (mode == _MODE_START_UP) {
      commandExecuted = true;
    }

  } else if (len == MGMLIS3MDL::TEMPERATURE_REPLY_LEN) {
    *foundLen = len;
    *foundId = MGMLIS3MDL::READ_TEMPERATURE;
  } else if (len == MGMLIS3MDL::SETUP_REPLY_LEN) {
    *foundLen = len;
    *foundId = MGMLIS3MDL::SETUP_MGM;
  } else if (len == SINGLE_COMMAND_ANSWER_LEN) {
    *foundLen = len;
    *foundId = getPendingCommand();
    if (*foundId == MGMLIS3MDL::IDENTIFY_DEVICE) {
      if (start[1] != MGMLIS3MDL::DEVICE_ID) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "MGMHandlerLIS3MDL::scanForReply: "
                        "Device identification failed!"
                     << std::endl;
#else
        sif::printWarning(
            "MGMHandlerLIS3MDL::scanForReply: "
            "Device identification failed!\n");
#endif
#endif
        return DeviceHandlerIF::INVALID_DATA;
      }

      if (mode == _MODE_START_UP) {
        commandExecuted = true;
      }
    }
  } else {
    return DeviceHandlerIF::INVALID_DATA;
  }

  /* Data with SPI Interface always has this answer */
  if (start[0] == 0b11111111) {
    return returnvalue::OK;
  } else {
    return DeviceHandlerIF::INVALID_DATA;
  }
}
ReturnValue_t MgmLIS3MDLHandler::interpretDeviceReply(DeviceCommandId_t id, const uint8_t *packet) {
  switch (id) {
    case MGMLIS3MDL::IDENTIFY_DEVICE: {
      break;
    }
    case MGMLIS3MDL::SETUP_MGM: {
      break;
    }
    case MGMLIS3MDL::READ_CONFIG_AND_DATA: {
      // TODO: Store configuration in new local datasets.
      float sensitivityFactor = getSensitivityFactor(getSensitivity(registers[2]));

      int16_t mgmMeasurementRawX =
          packet[MGMLIS3MDL::X_HIGHBYTE_IDX] << 8 | packet[MGMLIS3MDL::X_LOWBYTE_IDX];
      int16_t mgmMeasurementRawY =
          packet[MGMLIS3MDL::Y_HIGHBYTE_IDX] << 8 | packet[MGMLIS3MDL::Y_LOWBYTE_IDX];
      int16_t mgmMeasurementRawZ =
          packet[MGMLIS3MDL::Z_HIGHBYTE_IDX] << 8 | packet[MGMLIS3MDL::Z_LOWBYTE_IDX];

      // Target value in microtesla
      float mgmX = static_cast<float>(mgmMeasurementRawX) * sensitivityFactor *
                   MGMLIS3MDL::GAUSS_TO_MICROTESLA_FACTOR;
      float mgmY = static_cast<float>(mgmMeasurementRawY) * sensitivityFactor *
                   MGMLIS3MDL::GAUSS_TO_MICROTESLA_FACTOR;
      float mgmZ = static_cast<float>(mgmMeasurementRawZ) * sensitivityFactor *
                   MGMLIS3MDL::GAUSS_TO_MICROTESLA_FACTOR;

      if (periodicPrintout) {
        if (debugDivider.checkAndIncrement()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
          sif::info << "MGMHandlerLIS3: Magnetic field strength in"
                       " microtesla:"
                    << std::endl;
          sif::info << "X: " << mgmX << " uT" << std::endl;
          sif::info << "Y: " << mgmY << " uT" << std::endl;
          sif::info << "Z: " << mgmZ << " uT" << std::endl;
#else
          sif::printInfo("MGMHandlerLIS3: Magnetic field strength in microtesla:\n");
          sif::printInfo("X: %f uT\n", mgmX);
          sif::printInfo("Y: %f uT\n", mgmY);
          sif::printInfo("Z: %f uT\n", mgmZ);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 0 */
        }
      }

      PoolReadGuard readHelper(&dataset);
      if (readHelper.getReadResult() == returnvalue::OK) {
        if (std::abs(mgmX) > absLimitX or std::abs(mgmY) > absLimitY or
            std::abs(mgmZ) > absLimitZ) {
          dataset.fieldStrengths.setValid(false);
        }
        if (std::abs(mgmX) < absLimitX) {
          dataset.fieldStrengths[0] = mgmX;
        }

        if (std::abs(mgmY) < absLimitY) {
          dataset.fieldStrengths[1] = mgmY;
        }

        if (std::abs(mgmZ) < absLimitZ) {
          dataset.fieldStrengths[2] = mgmZ;
        }
        dataset.fieldStrengths.setValid(true);
      }
      break;
    }

    case MGMLIS3MDL::READ_TEMPERATURE: {
      int16_t tempValueRaw = packet[2] << 8 | packet[1];
      float tempValue = 25.0 + ((static_cast<float>(tempValueRaw)) / 8.0);
      if (periodicPrintout) {
        if (debugDivider.check()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
          sif::info << "MGMHandlerLIS3: Temperature: " << tempValue << " C" << std::endl;
#else
          sif::printInfo("MGMHandlerLIS3: Temperature: %f C\n");
#endif
        }
      }

      ReturnValue_t result = dataset.read();
      if (result == returnvalue::OK) {
        dataset.temperature = tempValue;
        dataset.commit();
      }
      break;
    }

    default: {
      return DeviceHandlerIF::UNKNOWN_DEVICE_REPLY;
    }
  }
  return returnvalue::OK;
}

MGMLIS3MDL::Sensitivies MgmLIS3MDLHandler::getSensitivity(uint8_t ctrlRegister2) {
  bool fs0Set = ctrlRegister2 & (1 << MGMLIS3MDL::FSO);  // Checks if FS0 bit is set
  bool fs1Set = ctrlRegister2 & (1 << MGMLIS3MDL::FS1);  // Checks if FS1 bit is set

  if (fs0Set && fs1Set)
    return MGMLIS3MDL::Sensitivies::GAUSS_16;
  else if (!fs0Set && fs1Set)
    return MGMLIS3MDL::Sensitivies::GAUSS_12;
  else if (fs0Set && !fs1Set)
    return MGMLIS3MDL::Sensitivies::GAUSS_8;
  else
    return MGMLIS3MDL::Sensitivies::GAUSS_4;
}

float MgmLIS3MDLHandler::getSensitivityFactor(MGMLIS3MDL::Sensitivies sens) {
  switch (sens) {
    case (MGMLIS3MDL::GAUSS_4): {
      return MGMLIS3MDL::FIELD_LSB_PER_GAUSS_4_SENS;
    }
    case (MGMLIS3MDL::GAUSS_8): {
      return MGMLIS3MDL::FIELD_LSB_PER_GAUSS_8_SENS;
    }
    case (MGMLIS3MDL::GAUSS_12): {
      return MGMLIS3MDL::FIELD_LSB_PER_GAUSS_12_SENS;
    }
    case (MGMLIS3MDL::GAUSS_16): {
      return MGMLIS3MDL::FIELD_LSB_PER_GAUSS_16_SENS;
    }
    default: {
      // Should never happen
      return MGMLIS3MDL::FIELD_LSB_PER_GAUSS_4_SENS;
    }
  }
}

ReturnValue_t MgmLIS3MDLHandler::enableTemperatureSensor(const uint8_t *commandData,
                                                         size_t commandDataLen) {
  triggerEvent(CHANGE_OF_SETUP_PARAMETER);
  uint32_t size = 2;
  commandBuffer[0] = writeCommand(MGMLIS3MDL::CTRL_REG1);
  if (commandDataLen > 1) {
    return INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
  }
  switch (*commandData) {
    case (MGMLIS3MDL::ON): {
      commandBuffer[1] = registers[0] | (1 << 7);
      break;
    }
    case (MGMLIS3MDL::OFF): {
      commandBuffer[1] = registers[0] & ~(1 << 7);
      break;
    }
    default:
      return INVALID_COMMAND_PARAMETER;
  }
  registers[0] = commandBuffer[1];

  rawPacket = commandBuffer;
  rawPacketLen = size;

  return returnvalue::OK;
}

ReturnValue_t MgmLIS3MDLHandler::setOperatingMode(const uint8_t *commandData,
                                                  size_t commandDataLen) {
  triggerEvent(CHANGE_OF_SETUP_PARAMETER);
  if (commandDataLen != 1) {
    return INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
  }

  switch (commandData[0]) {
    case MGMLIS3MDL::LOW:
      registers[0] = (registers[0] & (~(1 << MGMLIS3MDL::OM1))) & (~(1 << MGMLIS3MDL::OM0));
      registers[3] = (registers[3] & (~(1 << MGMLIS3MDL::OMZ1))) & (~(1 << MGMLIS3MDL::OMZ0));
      break;
    case MGMLIS3MDL::MEDIUM:
      registers[0] = (registers[0] & (~(1 << MGMLIS3MDL::OM1))) | (1 << MGMLIS3MDL::OM0);
      registers[3] = (registers[3] & (~(1 << MGMLIS3MDL::OMZ1))) | (1 << MGMLIS3MDL::OMZ0);
      break;

    case MGMLIS3MDL::HIGH:
      registers[0] = (registers[0] | (1 << MGMLIS3MDL::OM1)) & (~(1 << MGMLIS3MDL::OM0));
      registers[3] = (registers[3] | (1 << MGMLIS3MDL::OMZ1)) & (~(1 << MGMLIS3MDL::OMZ0));
      break;

    case MGMLIS3MDL::ULTRA:
      registers[0] = (registers[0] | (1 << MGMLIS3MDL::OM1)) | (1 << MGMLIS3MDL::OM0);
      registers[3] = (registers[3] | (1 << MGMLIS3MDL::OMZ1)) | (1 << MGMLIS3MDL::OMZ0);
      break;
    default:
      break;
  }

  return prepareCtrlRegisterWrite();
}

void MgmLIS3MDLHandler::fillCommandAndReplyMap() {
  insertInCommandAndReplyMap(MGMLIS3MDL::READ_CONFIG_AND_DATA, 1, &dataset);
  insertInCommandAndReplyMap(MGMLIS3MDL::READ_TEMPERATURE, 1);
  insertInCommandAndReplyMap(MGMLIS3MDL::SETUP_MGM, 1);
  insertInCommandAndReplyMap(MGMLIS3MDL::IDENTIFY_DEVICE, 1);
  insertInCommandAndReplyMap(MGMLIS3MDL::TEMP_SENSOR_ENABLE, 1);
  insertInCommandAndReplyMap(MGMLIS3MDL::ACCURACY_OP_MODE_SET, 1);
}

void MgmLIS3MDLHandler::setToGoToNormalMode(bool enable) { this->goToNormalMode = enable; }

ReturnValue_t MgmLIS3MDLHandler::prepareCtrlRegisterWrite() {
  commandBuffer[0] = writeCommand(MGMLIS3MDL::CTRL_REG1, true);

  for (size_t i = 0; i < MGMLIS3MDL::NR_OF_CTRL_REGISTERS; i++) {
    commandBuffer[i + 1] = registers[i];
  }
  rawPacket = commandBuffer;
  rawPacketLen = MGMLIS3MDL::NR_OF_CTRL_REGISTERS + 1;

  // We dont have to check if this is working because we just did i
  return returnvalue::OK;
}

void MgmLIS3MDLHandler::doTransition(Mode_t modeFrom, Submode_t subModeFrom) {
  DeviceHandlerBase::doTransition(modeFrom, subModeFrom);
}

uint32_t MgmLIS3MDLHandler::getTransitionDelayMs(Mode_t from, Mode_t to) { return transitionDelay; }

void MgmLIS3MDLHandler::modeChanged(void) { internalState = InternalState::STATE_NONE; }

ReturnValue_t MgmLIS3MDLHandler::initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
                                                         LocalDataPoolManager &poolManager) {
  localDataPoolMap.emplace(MGMLIS3MDL::FIELD_STRENGTHS, &mgmXYZ);
  localDataPoolMap.emplace(MGMLIS3MDL::TEMPERATURE_CELCIUS, &temperature);
  poolManager.subscribeForRegularPeriodicPacket({dataset.getSid(), false, 10.0});
  return returnvalue::OK;
}

void MgmLIS3MDLHandler::setAbsoluteLimits(float xLimit, float yLimit, float zLimit) {
  this->absLimitX = xLimit;
  this->absLimitY = yLimit;
  this->absLimitZ = zLimit;
}

void MgmLIS3MDLHandler::enablePeriodicPrintouts(bool enable, uint8_t divider) {
  periodicPrintout = enable;
  debugDivider.setDivider(divider);
}
