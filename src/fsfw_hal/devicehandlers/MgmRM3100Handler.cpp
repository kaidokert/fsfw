#include "MgmRM3100Handler.h"

#include "fsfw/datapool/PoolReadGuard.h"
#include "fsfw/devicehandlers/DeviceHandlerMessage.h"
#include "fsfw/globalfunctions/bitutility.h"
#include "fsfw/objectmanager/SystemObjectIF.h"
#include "fsfw/returnvalues/returnvalue.h"

MgmRM3100Handler::MgmRM3100Handler(object_id_t objectId, object_id_t deviceCommunication,
                                   CookieIF *comCookie, uint32_t transitionDelay)
    : DeviceHandlerBase(objectId, deviceCommunication, comCookie),
      primaryDataset(this),
      transitionDelay(transitionDelay) {}

MgmRM3100Handler::~MgmRM3100Handler() {}

void MgmRM3100Handler::doStartUp() {
  switch (internalState) {
    case (InternalState::NONE): {
      internalState = InternalState::CONFIGURE_CMM;
      break;
    }
    case (InternalState::CONFIGURE_CMM): {
      internalState = InternalState::READ_CMM;
      break;
    }
    case (InternalState::READ_CMM): {
      if (commandExecuted) {
        internalState = InternalState::STATE_CONFIGURE_TMRC;
      }
      break;
    }
    case (InternalState::STATE_CONFIGURE_TMRC): {
      if (commandExecuted) {
        internalState = InternalState::STATE_READ_TMRC;
      }
      break;
    }
    case (InternalState::STATE_READ_TMRC): {
      if (commandExecuted) {
        internalState = InternalState::NORMAL;
        if (goToNormalModeAtStartup) {
          setMode(MODE_NORMAL);
        } else {
          setMode(_MODE_TO_ON);
        }
      }
      break;
    }
    default: {
      break;
    }
  }
}

void MgmRM3100Handler::doShutDown() { setMode(_MODE_POWER_DOWN); }

ReturnValue_t MgmRM3100Handler::buildTransitionDeviceCommand(DeviceCommandId_t *id) {
  size_t commandLen = 0;
  switch (internalState) {
    case (InternalState::NONE):
    case (InternalState::NORMAL): {
      return NOTHING_TO_SEND;
    }
    case (InternalState::CONFIGURE_CMM): {
      *id = RM3100::CONFIGURE_CMM;
      break;
    }
    case (InternalState::READ_CMM): {
      *id = RM3100::READ_CMM;
      break;
    }
    case (InternalState::STATE_CONFIGURE_TMRC): {
      commandBuffer[0] = RM3100::TMRC_DEFAULT_VALUE;
      commandLen = 1;
      *id = RM3100::CONFIGURE_TMRC;
      break;
    }
    case (InternalState::STATE_READ_TMRC): {
      *id = RM3100::READ_TMRC;
      break;
    }
    default:
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      // Might be a configuration error
      sif::warning << "MgmRM3100Handler::buildTransitionDeviceCommand: "
                      "Unknown internal state"
                   << std::endl;
#else
      sif::printWarning(
          "MgmRM3100Handler::buildTransitionDeviceCommand: "
          "Unknown internal state\n");
#endif
#endif
      return returnvalue::OK;
  }

  return buildCommandFromCommand(*id, commandBuffer, commandLen);
}

ReturnValue_t MgmRM3100Handler::buildCommandFromCommand(DeviceCommandId_t deviceCommand,
                                                        const uint8_t *commandData,
                                                        size_t commandDataLen) {
  switch (deviceCommand) {
    case (RM3100::CONFIGURE_CMM): {
      commandBuffer[0] = RM3100::CMM_REGISTER;
      commandBuffer[1] = RM3100::CMM_VALUE;
      rawPacket = commandBuffer;
      rawPacketLen = 2;
      break;
    }
    case (RM3100::READ_CMM): {
      commandBuffer[0] = RM3100::CMM_REGISTER | RM3100::READ_MASK;
      commandBuffer[1] = 0;
      rawPacket = commandBuffer;
      rawPacketLen = 2;
      break;
    }
    case (RM3100::CONFIGURE_TMRC): {
      return handleTmrcConfigCommand(deviceCommand, commandData, commandDataLen);
    }
    case (RM3100::READ_TMRC): {
      commandBuffer[0] = RM3100::TMRC_REGISTER | RM3100::READ_MASK;
      commandBuffer[1] = 0;
      rawPacket = commandBuffer;
      rawPacketLen = 2;
      break;
    }
    case (RM3100::CONFIGURE_CYCLE_COUNT): {
      return handleCycleCountConfigCommand(deviceCommand, commandData, commandDataLen);
    }
    case (RM3100::READ_CYCLE_COUNT): {
      commandBuffer[0] = RM3100::CYCLE_COUNT_START_REGISTER | RM3100::READ_MASK;
      std::memset(commandBuffer + 1, 0, 6);
      rawPacket = commandBuffer;
      rawPacketLen = 7;
      break;
    }
    case (RM3100::READ_DATA): {
      commandBuffer[0] = RM3100::MEASUREMENT_REG_START | RM3100::READ_MASK;
      std::memset(commandBuffer + 1, 0, 9);
      rawPacketLen = 10;
      break;
    }
    default:
      return DeviceHandlerIF::COMMAND_NOT_IMPLEMENTED;
  }
  return returnvalue::OK;
}

ReturnValue_t MgmRM3100Handler::buildNormalDeviceCommand(DeviceCommandId_t *id) {
  *id = RM3100::READ_DATA;
  return buildCommandFromCommand(*id, nullptr, 0);
}

ReturnValue_t MgmRM3100Handler::scanForReply(const uint8_t *start, size_t len,
                                             DeviceCommandId_t *foundId, size_t *foundLen) {
  // For SPI, ID will always be the one of the last sent command
  *foundId = this->getPendingCommand();
  *foundLen = len;
  return returnvalue::OK;
}

ReturnValue_t MgmRM3100Handler::interpretDeviceReply(DeviceCommandId_t id, const uint8_t *packet) {
  ReturnValue_t result = returnvalue::OK;
  switch (id) {
    case (RM3100::CONFIGURE_CMM):
    case (RM3100::CONFIGURE_CYCLE_COUNT):
    case (RM3100::CONFIGURE_TMRC): {
      // We can only check whether write was successful with read operation
      if (mode == _MODE_START_UP) {
        commandExecuted = true;
      }
      break;
    }
    case (RM3100::READ_CMM): {
      uint8_t cmmValue = packet[1];
      // We clear the seventh bit in any case
      // because this one is zero sometimes for some reason
      bitutil::clear(&cmmValue, 6);
      if (cmmValue == cmmRegValue and internalState == InternalState::READ_CMM) {
        commandExecuted = true;
      } else {
        // Attempt reconfiguration
        internalState = InternalState::CONFIGURE_CMM;
        return DeviceHandlerIF::DEVICE_REPLY_INVALID;
      }
      break;
    }
    case (RM3100::READ_TMRC): {
      if (packet[1] == tmrcRegValue) {
        commandExecuted = true;
        // Reading TMRC was commanded. Trigger event to inform ground
        if (mode != _MODE_START_UP) {
          triggerEvent(tmrcSet, tmrcRegValue, 0);
        }
      } else {
        // Attempt reconfiguration
        internalState = InternalState::STATE_CONFIGURE_TMRC;
        return DeviceHandlerIF::DEVICE_REPLY_INVALID;
      }
      break;
    }
    case (RM3100::READ_CYCLE_COUNT): {
      uint16_t cycleCountX = packet[1] << 8 | packet[2];
      uint16_t cycleCountY = packet[3] << 8 | packet[4];
      uint16_t cycleCountZ = packet[5] << 8 | packet[6];
      if (cycleCountX != cycleCountRegValueX or cycleCountY != cycleCountRegValueY or
          cycleCountZ != cycleCountRegValueZ) {
        return DeviceHandlerIF::DEVICE_REPLY_INVALID;
      }
      // Reading TMRC was commanded. Trigger event to inform ground
      if (mode != _MODE_START_UP) {
        uint32_t eventParam1 = (cycleCountX << 16) | cycleCountY;
        triggerEvent(cycleCountersSet, eventParam1, cycleCountZ);
      }
      break;
    }
    case (RM3100::READ_DATA): {
      result = handleDataReadout(packet);
      break;
    }
    default:
      return DeviceHandlerIF::UNKNOWN_DEVICE_REPLY;
  }

  return result;
}

ReturnValue_t MgmRM3100Handler::handleCycleCountConfigCommand(DeviceCommandId_t deviceCommand,
                                                              const uint8_t *commandData,
                                                              size_t commandDataLen) {
  if (commandData == nullptr) {
    return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
  }

  // Set cycle count
  if (commandDataLen == 2) {
    handleCycleCommand(true, commandData, commandDataLen);
  } else if (commandDataLen == 6) {
    handleCycleCommand(false, commandData, commandDataLen);
  } else {
    return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
  }

  commandBuffer[0] = RM3100::CYCLE_COUNT_VALUE;
  std::memcpy(commandBuffer + 1, &cycleCountRegValueX, 2);
  std::memcpy(commandBuffer + 3, &cycleCountRegValueY, 2);
  std::memcpy(commandBuffer + 5, &cycleCountRegValueZ, 2);
  rawPacketLen = 7;
  rawPacket = commandBuffer;
  return returnvalue::OK;
}

ReturnValue_t MgmRM3100Handler::handleCycleCommand(bool oneCycleValue, const uint8_t *commandData,
                                                   size_t commandDataLen) {
  RM3100::CycleCountCommand command(oneCycleValue);
  ReturnValue_t result =
      command.deSerialize(&commandData, &commandDataLen, SerializeIF::Endianness::BIG);
  if (result != returnvalue::OK) {
    return result;
  }

  // Data sheet p.30 "while noise limits the useful upper range to ~400 cycle counts."
  if (command.cycleCountX > 450) {
    return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
  }

  if (not oneCycleValue and (command.cycleCountY > 450 or command.cycleCountZ > 450)) {
    return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
  }

  cycleCountRegValueX = command.cycleCountX;
  cycleCountRegValueY = command.cycleCountY;
  cycleCountRegValueZ = command.cycleCountZ;
  return returnvalue::OK;
}

ReturnValue_t MgmRM3100Handler::handleTmrcConfigCommand(DeviceCommandId_t deviceCommand,
                                                        const uint8_t *commandData,
                                                        size_t commandDataLen) {
  if (commandData == nullptr or commandDataLen != 1) {
    return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
  }

  commandBuffer[0] = RM3100::TMRC_REGISTER;
  commandBuffer[1] = commandData[0];
  tmrcRegValue = commandData[0];
  rawPacketLen = 2;
  rawPacket = commandBuffer;
  return returnvalue::OK;
}

void MgmRM3100Handler::fillCommandAndReplyMap() {
  insertInCommandAndReplyMap(RM3100::CONFIGURE_CMM, 3);
  insertInCommandAndReplyMap(RM3100::READ_CMM, 3);

  insertInCommandAndReplyMap(RM3100::CONFIGURE_TMRC, 3);
  insertInCommandAndReplyMap(RM3100::READ_TMRC, 3);

  insertInCommandAndReplyMap(RM3100::CONFIGURE_CYCLE_COUNT, 3);
  insertInCommandAndReplyMap(RM3100::READ_CYCLE_COUNT, 3);

  insertInCommandAndReplyMap(RM3100::READ_DATA, 3, &primaryDataset);
}

void MgmRM3100Handler::modeChanged() { internalState = InternalState::NONE; }

ReturnValue_t MgmRM3100Handler::initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
                                                        LocalDataPoolManager &poolManager) {
  localDataPoolMap.emplace(RM3100::FIELD_STRENGTHS, &mgmXYZ);
  poolManager.subscribeForRegularPeriodicPacket({primaryDataset.getSid(), false, 10.0});
  return returnvalue::OK;
}

uint32_t MgmRM3100Handler::getTransitionDelayMs(Mode_t from, Mode_t to) {
  return this->transitionDelay;
}

void MgmRM3100Handler::setToGoToNormalMode(bool enable) { goToNormalModeAtStartup = enable; }

ReturnValue_t MgmRM3100Handler::handleDataReadout(const uint8_t *packet) {
  // Analyze data here. The sensor generates 24 bit signed values so we need to do some bitshift
  // trickery here to calculate the raw values first
  int32_t fieldStrengthRawX = ((packet[1] << 24) | (packet[2] << 16) | (packet[3] << 8)) >> 8;
  int32_t fieldStrengthRawY = ((packet[4] << 24) | (packet[5] << 16) | (packet[6] << 8)) >> 8;
  int32_t fieldStrengthRawZ = ((packet[7] << 24) | (packet[8] << 16) | (packet[3] << 8)) >> 8;

  // Now scale to physical value in microtesla
  float fieldStrengthX = fieldStrengthRawX * scaleFactorX;
  float fieldStrengthY = fieldStrengthRawY * scaleFactorX;
  float fieldStrengthZ = fieldStrengthRawZ * scaleFactorX;

  if (periodicPrintout) {
    if (debugDivider.checkAndIncrement()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::info << "MgmRM3100Handler: Magnetic field strength in"
                   " microtesla:"
                << std::endl;
      sif::info << "X: " << fieldStrengthX << " uT" << std::endl;
      sif::info << "Y: " << fieldStrengthY << " uT" << std::endl;
      sif::info << "Z: " << fieldStrengthZ << " uT" << std::endl;
#else
      sif::printInfo("MgmRM3100Handler: Magnetic field strength in microtesla:\n");
      sif::printInfo("X: %f uT\n", fieldStrengthX);
      sif::printInfo("Y: %f uT\n", fieldStrengthY);
      sif::printInfo("Z: %f uT\n", fieldStrengthZ);
#endif
    }
  }

  // TODO: Sanity check on values?
  PoolReadGuard readGuard(&primaryDataset);
  if (readGuard.getReadResult() == returnvalue::OK) {
    primaryDataset.fieldStrengths[0] = fieldStrengthX;
    primaryDataset.fieldStrengths[1] = fieldStrengthY;
    primaryDataset.fieldStrengths[2] = fieldStrengthZ;
    primaryDataset.setValidity(true, true);
  }
  return returnvalue::OK;
}

void MgmRM3100Handler::enablePeriodicPrintouts(bool enable, uint8_t divider) {
  periodicPrintout = enable;
  debugDivider.setDivider(divider);
}
