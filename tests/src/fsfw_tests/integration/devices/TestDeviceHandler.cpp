#include "TestDeviceHandler.h"

#include <cstdlib>

#include "FSFWConfig.h"
#include "fsfw/datapool/PoolReadGuard.h"

TestDevice::TestDevice(object_id_t objectId, object_id_t comIF, CookieIF* cookie,
                       testdevice::DeviceIndex deviceIdx, bool fullInfoPrintout,
                       bool changingDataset)
    : DeviceHandlerBase(objectId, comIF, cookie),
      deviceIdx(deviceIdx),
      dataset(this),
      fullInfoPrintout(fullInfoPrintout) {}

TestDevice::~TestDevice() = default;

void TestDevice::performOperationHook() {
  if (periodicPrintout) {
    FSFW_LOGI("TestDevice {} | performOperationHook: Alive!\n", deviceIdx);
  }

  if (oneShot) {
    oneShot = false;
  }
}

void TestDevice::doStartUp() {
  if (fullInfoPrintout) {
    FSFW_LOGI("TestDevice {} | doStartUp: Switching On\n", deviceIdx);
  }

  setMode(_MODE_TO_ON);
}

void TestDevice::doShutDown() {
  if (fullInfoPrintout) {
    FSFW_LOGI("TestDevice {} | doShutDown: Switching Off\n", deviceIdx);
  }

  setMode(_MODE_SHUT_DOWN);
}

ReturnValue_t TestDevice::buildNormalDeviceCommand(DeviceCommandId_t* id) {
  using namespace testdevice;
  *id = TEST_NORMAL_MODE_CMD;
  if (DeviceHandlerBase::isAwaitingReply()) {
    return NOTHING_TO_SEND;
  }
  return buildCommandFromCommand(*id, nullptr, 0);
}

ReturnValue_t TestDevice::buildTransitionDeviceCommand(DeviceCommandId_t* id) {
  if (mode == _MODE_TO_ON) {
    if (fullInfoPrintout) {
      FSFW_LOGI(
          "TestDevice {} | buildTransitionDeviceCommand: Was called"
          " from _MODE_TO_ON mode\n",
          deviceIdx);
    }
  }
  if (mode == _MODE_TO_NORMAL) {
    if (fullInfoPrintout) {
      FSFW_LOGI(
          "TestDevice {} | buildTransitionDeviceCommand: Was called "
          "from _MODE_TO_NORMAL mode\n",
          deviceIdx);
    }

    setMode(MODE_NORMAL);
  }
  if (mode == _MODE_SHUT_DOWN) {
    if (fullInfoPrintout) {
      FSFW_LOGI(
          "TestDevice {} | buildTransitionDeviceCommand: Was called "
          "from _MODE_SHUT_DOWN mode\n",
          deviceIdx);
    }

    setMode(MODE_OFF);
  }
  return NOTHING_TO_SEND;
}

void TestDevice::doTransition(Mode_t modeFrom, Submode_t submodeFrom) {
  if (mode == _MODE_TO_NORMAL) {
    if (fullInfoPrintout) {
      FSFW_LOGI(
          "TestDevice {} | doTransition: Custom transition to "
          "normal mode\n",
          deviceIdx);
    }

  } else {
    DeviceHandlerBase::doTransition(modeFrom, submodeFrom);
  }
}

ReturnValue_t TestDevice::buildCommandFromCommand(DeviceCommandId_t deviceCommand,
                                                  const uint8_t* commandData,
                                                  size_t commandDataLen) {
  using namespace testdevice;
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  switch (deviceCommand) {
    case (TEST_NORMAL_MODE_CMD): {
      commandSent = true;
      result = buildNormalModeCommand(deviceCommand, commandData, commandDataLen);
      break;
    }

    case (TEST_COMMAND_0): {
      commandSent = true;
      result = buildTestCommand0(deviceCommand, commandData, commandDataLen);
      break;
    }

    case (TEST_COMMAND_1): {
      commandSent = true;
      result = buildTestCommand1(deviceCommand, commandData, commandDataLen);
      break;
    }
    case (TEST_NOTIF_SNAPSHOT_VAR): {
      if (changingDatasets) {
        changingDatasets = false;
      }

      PoolReadGuard readHelper(&dataset.testUint8Var);
      if (deviceIdx == testdevice::DeviceIndex::DEVICE_0) {
        /* This will trigger a variable notification to the demo controller */
        dataset.testUint8Var = 220;
        dataset.testUint8Var.setValid(true);
      } else if (deviceIdx == testdevice::DeviceIndex::DEVICE_1) {
        /* This will trigger a variable snapshot to the demo controller */
        dataset.testUint8Var = 30;
        dataset.testUint8Var.setValid(true);
      }

      break;
    }
    case (TEST_NOTIF_SNAPSHOT_SET): {
      if (changingDatasets) {
        changingDatasets = false;
      }

      PoolReadGuard readHelper(&dataset.testFloat3Vec);

      if (deviceIdx == testdevice::DeviceIndex::DEVICE_0) {
        /* This will trigger a variable notification to the demo controller */
        dataset.testFloat3Vec.value[0] = 60;
        dataset.testFloat3Vec.value[1] = 70;
        dataset.testFloat3Vec.value[2] = 55;
        dataset.testFloat3Vec.setValid(true);
      } else if (deviceIdx == testdevice::DeviceIndex::DEVICE_1) {
        /* This will trigger a variable notification to the demo controller */
        dataset.testFloat3Vec.value[0] = -60;
        dataset.testFloat3Vec.value[1] = -70;
        dataset.testFloat3Vec.value[2] = -55;
        dataset.testFloat3Vec.setValid(true);
      }
      break;
    }
    default:
      result = DeviceHandlerIF::COMMAND_NOT_SUPPORTED;
  }
  return result;
}

ReturnValue_t TestDevice::buildNormalModeCommand(DeviceCommandId_t deviceCommand,
                                                 const uint8_t* commandData,
                                                 size_t commandDataLen) {
  if (fullInfoPrintout) {
#if FSFW_VERBOSE_LEVEL >= 3
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "TestDevice::buildTestCommand1: Building normal command" << std::endl;
#else
    sif::printInfo("TestDevice::buildTestCommand1: Building command from TEST_COMMAND_1\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* OBSW_VERBOSE_LEVEL >= 3 */
  }

  if (commandDataLen > MAX_BUFFER_SIZE - sizeof(DeviceCommandId_t)) {
    return DeviceHandlerIF::INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
  }
  /* The command is passed on in the command buffer as it is */
  passOnCommand(deviceCommand, commandData, commandDataLen);
  return RETURN_OK;
}

ReturnValue_t TestDevice::buildTestCommand0(DeviceCommandId_t deviceCommand,
                                            const uint8_t* commandData, size_t commandDataLen) {
  using namespace testdevice;
  if (fullInfoPrintout) {
    FSFW_LOGI(
        "TestDevice {} | buildTestCommand0: Executing simple command "
        "with completion reply\n",
        deviceIdx);
  }

  if (commandDataLen > MAX_BUFFER_SIZE - sizeof(DeviceCommandId_t)) {
    return DeviceHandlerIF::INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
  }

  /* The command is passed on in the command buffer as it is */
  passOnCommand(deviceCommand, commandData, commandDataLen);
  return RETURN_OK;
}

ReturnValue_t TestDevice::buildTestCommand1(DeviceCommandId_t deviceCommand,
                                            const uint8_t* commandData, size_t commandDataLen) {
  using namespace testdevice;
  if (commandDataLen < 7) {
    return DeviceHandlerIF::INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
  }
  if (fullInfoPrintout) {
    FSFW_LOGI(
        "TestDevice {} | buildTestCommand1: Executing command "
        "with data reply\n",
        deviceIdx);
  }

  deviceCommand = EndianConverter::convertBigEndian(deviceCommand);
  memcpy(commandBuffer, &deviceCommand, sizeof(deviceCommand));

  /* Assign and check parameters */
  uint16_t parameter1 = 0;
  size_t size = commandDataLen;
  ReturnValue_t result =
      SerializeAdapter::deSerialize(&parameter1, &commandData, &size, SerializeIF::Endianness::BIG);
  if (result == HasReturnvaluesIF::RETURN_FAILED) {
    return result;
  }

  /* Parameter 1 needs to be correct */
  if (parameter1 != testdevice::COMMAND_1_PARAM1) {
    return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
  }
  uint64_t parameter2 = 0;
  result =
      SerializeAdapter::deSerialize(&parameter2, &commandData, &size, SerializeIF::Endianness::BIG);
  if (parameter2 != testdevice::COMMAND_1_PARAM2) {
    return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
  }

  /* Pass on the parameters to the Echo IF */
  commandBuffer[4] = (parameter1 & 0xFF00) >> 8;
  commandBuffer[5] = (parameter1 & 0xFF);
  parameter2 = EndianConverter::convertBigEndian(parameter2);
  memcpy(commandBuffer + 6, &parameter2, sizeof(parameter2));
  rawPacket = commandBuffer;
  rawPacketLen = sizeof(deviceCommand) + sizeof(parameter1) + sizeof(parameter2);
  return RETURN_OK;
}

void TestDevice::passOnCommand(DeviceCommandId_t command, const uint8_t* commandData,
                               size_t commandDataLen) {
  DeviceCommandId_t deviceCommandBe = EndianConverter::convertBigEndian(command);
  memcpy(commandBuffer, &deviceCommandBe, sizeof(deviceCommandBe));
  memcpy(commandBuffer + 4, commandData, commandDataLen);
  rawPacket = commandBuffer;
  rawPacketLen = sizeof(deviceCommandBe) + commandDataLen;
}

void TestDevice::fillCommandAndReplyMap() {
  namespace td = testdevice;
  insertInCommandAndReplyMap(testdevice::TEST_NORMAL_MODE_CMD, 5, &dataset);
  insertInCommandAndReplyMap(testdevice::TEST_COMMAND_0, 5);
  insertInCommandAndReplyMap(testdevice::TEST_COMMAND_1, 5);

  /* No reply expected for these commands */
  insertInCommandMap(td::TEST_NOTIF_SNAPSHOT_SET);
  insertInCommandMap(td::TEST_NOTIF_SNAPSHOT_VAR);
}

ReturnValue_t TestDevice::scanForReply(const uint8_t* start, size_t len, DeviceCommandId_t* foundId,
                                       size_t* foundLen) {
  using namespace testdevice;

  /* Unless a command was sent explicitely, we don't expect any replies and ignore this
  the packet. On a real device, there might be replies which are sent without a previous
  command. */
  if (not commandSent) {
    return DeviceHandlerBase::IGNORE_FULL_PACKET;
  } else {
    commandSent = false;
  }

  if (len < sizeof(object_id_t)) {
    return DeviceHandlerIF::LENGTH_MISSMATCH;
  }

  size_t size = len;
  ReturnValue_t result =
      SerializeAdapter::deSerialize(foundId, &start, &size, SerializeIF::Endianness::BIG);
  if (result != RETURN_OK) {
    return result;
  }

  DeviceCommandId_t pendingCmd = this->getPendingCommand();

  switch (pendingCmd) {
    case (TEST_NORMAL_MODE_CMD): {
      if (fullInfoPrintout) {
#if FSFW_VERBOSE_LEVEL >= 3
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "TestDevice::scanForReply: Reply for normal commnand (ID "
                  << TEST_NORMAL_MODE_CMD << ") received!" << std::endl;
#else
        sif::printInfo(
            "TestDevice%d::scanForReply: Reply for normal command (ID %d) "
            "received!\n",
            deviceIdx, TEST_NORMAL_MODE_CMD);
#endif
#endif
      }

      *foundLen = len;
      *foundId = pendingCmd;
      return RETURN_OK;
    }

    case (TEST_COMMAND_0): {
      if (len < TEST_COMMAND_0_SIZE) {
        return DeviceHandlerIF::LENGTH_MISSMATCH;
      }
      if (fullInfoPrintout) {
        FSFW_LOGIT("TestDevice {} | scanForReply: Reply for simple command (ID {}) received!\n",
                   deviceIdx, TEST_COMMAND_0);
      }

      *foundLen = TEST_COMMAND_0_SIZE;
      *foundId = pendingCmd;
      return RETURN_OK;
    }

    case (TEST_COMMAND_1): {
      if (fullInfoPrintout) {
        FSFW_LOGIT("TestDevice {} | scanForReply: Reply for data command (ID {}) received\n",
                   deviceIdx, TEST_COMMAND_1);
      }

      *foundLen = len;
      *foundId = pendingCmd;
      return RETURN_OK;
    }

    default:
      return DeviceHandlerIF::DEVICE_REPLY_INVALID;
  }
}

ReturnValue_t TestDevice::interpretDeviceReply(DeviceCommandId_t id, const uint8_t* packet) {
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  switch (id) {
    /* Periodic replies */
    case testdevice::TEST_NORMAL_MODE_CMD: {
      result = interpretingNormalModeReply();
      break;
    }
    /* Simple reply */
    case testdevice::TEST_COMMAND_0: {
      result = interpretingTestReply0(id, packet);
      break;
    }
    /* Data reply */
    case testdevice::TEST_COMMAND_1: {
      result = interpretingTestReply1(id, packet);
      break;
    }
    default:
      return DeviceHandlerIF::DEVICE_REPLY_INVALID;
  }
  return result;
}

ReturnValue_t TestDevice::interpretingNormalModeReply() {
  CommandMessage directReplyMessage;
  if (changingDatasets) {
    PoolReadGuard readHelper(&dataset);
    if (dataset.testUint8Var.value == 0) {
      dataset.testUint8Var.value = 10;
      dataset.testUint32Var.value = 777;
      dataset.testFloat3Vec.value[0] = 2.5;
      dataset.testFloat3Vec.value[1] = -2.5;
      dataset.testFloat3Vec.value[2] = 2.5;
      dataset.setValidity(true, true);
    } else {
      dataset.testUint8Var.value = 0;
      dataset.testUint32Var.value = 0;
      dataset.testFloat3Vec.value[0] = 0.0;
      dataset.testFloat3Vec.value[1] = 0.0;
      dataset.testFloat3Vec.value[2] = 0.0;
      dataset.setValidity(false, true);
    }
    return RETURN_OK;
  }

  PoolReadGuard readHelper(&dataset);
  if (dataset.testUint8Var.value == 0) {
    /* Reset state */
    dataset.testUint8Var.value = 128;
  } else if (dataset.testUint8Var.value > 200) {
    if (not resetAfterChange) {
      /* This will trigger an update notification to the controller */
      dataset.testUint8Var.setChanged(true);
      resetAfterChange = true;
      /* Decrement by 30 automatically. This will prevent any additional notifications. */
      dataset.testUint8Var.value -= 30;
    }
  }
  /* If the value is greater than 0, it will be decremented in a linear way */
  else if (dataset.testUint8Var.value > 128) {
    size_t sizeToDecrement = 0;
    if (dataset.testUint8Var.value > 128 + 30) {
      sizeToDecrement = 30;
    } else {
      sizeToDecrement = dataset.testUint8Var.value - 128;
      resetAfterChange = false;
    }
    dataset.testUint8Var.value -= sizeToDecrement;
  } else if (dataset.testUint8Var.value < 50) {
    if (not resetAfterChange) {
      /* This will trigger an update snapshot to the controller */
      dataset.testUint8Var.setChanged(true);
      resetAfterChange = true;
    } else {
      /* Increment by 30 automatically. */
      dataset.testUint8Var.value += 30;
    }
  }
  /* Increment in linear way */
  else if (dataset.testUint8Var.value < 128) {
    size_t sizeToIncrement = 0;
    if (dataset.testUint8Var.value < 128 - 20) {
      sizeToIncrement = 20;
    } else {
      sizeToIncrement = 128 - dataset.testUint8Var.value;
      resetAfterChange = false;
    }
    dataset.testUint8Var.value += sizeToIncrement;
  }

  /* TODO: Same for vector */
  float vectorMean = (dataset.testFloat3Vec.value[0] + dataset.testFloat3Vec.value[1] +
                      dataset.testFloat3Vec.value[2]) /
                     3.0;

  /* Lambda (private local function) */
  auto sizeToAdd = [](bool tooHigh, float currentVal) {
    if (tooHigh) {
      if (currentVal - 20.0 > 10.0) {
        return -10.0;
      } else {
        return 20.0 - currentVal;
      }
    } else {
      if (std::abs(currentVal + 20.0) > 10.0) {
        return 10.0;
      } else {
        return -20.0 - currentVal;
      }
    }
  };

  if (vectorMean > 20.0 and std::abs(vectorMean - 20.0) > 1.0) {
    if (not resetAfterChange) {
      dataset.testFloat3Vec.setChanged(true);
      resetAfterChange = true;
    } else {
      float sizeToDecrementVal0 = 0;
      float sizeToDecrementVal1 = 0;
      float sizeToDecrementVal2 = 0;

      sizeToDecrementVal0 = sizeToAdd(true, dataset.testFloat3Vec.value[0]);
      sizeToDecrementVal1 = sizeToAdd(true, dataset.testFloat3Vec.value[1]);
      sizeToDecrementVal2 = sizeToAdd(true, dataset.testFloat3Vec.value[2]);

      dataset.testFloat3Vec.value[0] += sizeToDecrementVal0;
      dataset.testFloat3Vec.value[1] += sizeToDecrementVal1;
      dataset.testFloat3Vec.value[2] += sizeToDecrementVal2;
    }
  } else if (vectorMean < -20.0 and std::abs(vectorMean + 20.0) < 1.0) {
    if (not resetAfterChange) {
      dataset.testFloat3Vec.setChanged(true);
      resetAfterChange = true;
    } else {
      float sizeToDecrementVal0 = 0;
      float sizeToDecrementVal1 = 0;
      float sizeToDecrementVal2 = 0;

      sizeToDecrementVal0 = sizeToAdd(false, dataset.testFloat3Vec.value[0]);
      sizeToDecrementVal1 = sizeToAdd(false, dataset.testFloat3Vec.value[1]);
      sizeToDecrementVal2 = sizeToAdd(false, dataset.testFloat3Vec.value[2]);

      dataset.testFloat3Vec.value[0] += sizeToDecrementVal0;
      dataset.testFloat3Vec.value[1] += sizeToDecrementVal1;
      dataset.testFloat3Vec.value[2] += sizeToDecrementVal2;
    }
  } else {
    if (resetAfterChange) {
      resetAfterChange = false;
    }
  }

  return RETURN_OK;
}

ReturnValue_t TestDevice::interpretingTestReply0(DeviceCommandId_t id, const uint8_t* packet) {
  CommandMessage commandMessage;
  if (fullInfoPrintout) {
    FSFW_LOGI("interpretingTestReply0: Generating step and finish reply\n");
  }

  MessageQueueId_t commander = getCommanderQueueId(id);
  /* Generate one step reply and the finish reply */
  actionHelper.step(1, commander, id);
  actionHelper.finish(true, commander, id);

  return RETURN_OK;
}

ReturnValue_t TestDevice::interpretingTestReply1(DeviceCommandId_t id, const uint8_t* packet) {
  CommandMessage directReplyMessage;
  if (fullInfoPrintout) {
    FSFW_LOGIT("TestDevice {} | interpretingReply1: Setting data reply\n", deviceIdx);
  }

  MessageQueueId_t commander = getCommanderQueueId(id);
  /* Send reply with data */
  ReturnValue_t result =
      actionHelper.reportData(commander, id, packet, testdevice::TEST_COMMAND_1_SIZE, false);

  if (result != RETURN_OK) {
    FSFW_LOGWT("TestDevice {} | interpretingReply1: Sending data reply failed\n", deviceIdx);
    /* Finish reply */
    actionHelper.finish(false, commander, id, result);
    return result;
  }

  /* Finish reply */
  actionHelper.finish(true, commander, id);
  return RETURN_OK;
}

uint32_t TestDevice::getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) { return 5000; }

void TestDevice::enableFullDebugOutput(bool enable) { this->fullInfoPrintout = enable; }

ReturnValue_t TestDevice::initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
                                                  LocalDataPoolManager& poolManager) {
  namespace td = testdevice;
  localDataPoolMap.emplace(td::PoolIds::TEST_UINT8_ID, new PoolEntry<uint8_t>({0}));
  localDataPoolMap.emplace(td::PoolIds::TEST_UINT32_ID, new PoolEntry<uint32_t>({0}));
  localDataPoolMap.emplace(td::PoolIds::TEST_FLOAT_VEC_3_ID, new PoolEntry<float>({0.0, 0.0, 0.0}));

  sid_t sid(this->getObjectId(), td::TEST_SET_ID);
  /* Subscribe for periodic HK packets but do not enable reporting for now.
  Non-diangostic with a period of one second */
  poolManager.subscribeForPeriodicPacket(sid, false, 1.0, false);
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TestDevice::getParameter(uint8_t domainId, uint8_t uniqueId,
                                       ParameterWrapper* parameterWrapper,
                                       const ParameterWrapper* newValues, uint16_t startAtIndex) {
  using namespace testdevice;
  switch (uniqueId) {
    case ParameterUniqueIds::TEST_UINT32_0: {
      if (fullInfoPrintout) {
        uint32_t newValue = 0;
        ReturnValue_t result = newValues->getElement<uint32_t>(&newValue, 0, 0);
        if (result == HasReturnvaluesIF::RETURN_OK) {
          FSFW_LOGIT("TestDevice {} | getParameter: Setting parameter 0 to new value {}\n",
                     newValue);
        }
      }
      parameterWrapper->set(testParameter0);
      break;
    }
    case ParameterUniqueIds::TEST_INT32_1: {
      if (fullInfoPrintout) {
        int32_t newValue = 0;
        ReturnValue_t result = newValues->getElement<int32_t>(&newValue, 0, 0);
        if (result == HasReturnvaluesIF::RETURN_OK) {
          FSFW_LOGIT("TestDevice {} | getParameter: Setting parameter 1 to new value {}\n",
                     newValue);
        }
      }
      parameterWrapper->set(testParameter1);
      break;
    }
    case ParameterUniqueIds::TEST_FLOAT_VEC3_2: {
      if (fullInfoPrintout) {
        float newVector[3];
        if (newValues->getElement<float>(newVector, 0, 0) != RETURN_OK or
            newValues->getElement<float>(newVector + 1, 0, 1) != RETURN_OK or
            newValues->getElement<float>(newVector + 2, 0, 2) != RETURN_OK) {
          return HasReturnvaluesIF::RETURN_FAILED;
        }
        FSFW_LOGIT(
            "TestDevice {} | getParameter: Setting parameter 3 (float vec with 3 entries) "
            "to new values 0 {}, 1 {}, 2 {}\n",
            newVector[0], newVector[1], newVector[2]);
      }
      parameterWrapper->setVector(vectorFloatParams2);
      break;
    }
    case (ParameterUniqueIds::PERIODIC_PRINT_ENABLED): {
      if (fullInfoPrintout) {
        uint8_t enabled = 0;
        ReturnValue_t result = newValues->getElement<uint8_t>(&enabled, 0, 0);
        if (result != HasReturnvaluesIF::RETURN_OK) {
          return result;
        }
        char const* printout = nullptr;
        if (enabled) {
          printout = "enabled";
        } else {
          printout = "disabled";
        }
        FSFW_LOGIT("TestDevice {} | getParameter: Periodic printout {}\n", deviceIdx, printout);
      }

      parameterWrapper->set(periodicPrintout);
      break;
    }
    case (ParameterUniqueIds::CHANGING_DATASETS): {
      uint8_t enabled = 0;
      ReturnValue_t result = newValues->getElement<uint8_t>(&enabled, 0, 0);
      if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
      }
      if (not enabled) {
        PoolReadGuard readHelper(&dataset);
        dataset.testUint8Var.value = 0;
        dataset.testUint32Var.value = 0;
        dataset.testFloat3Vec.value[0] = 0.0;
        dataset.testFloat3Vec.value[0] = 0.0;
        dataset.testFloat3Vec.value[1] = 0.0;
      }

      if (fullInfoPrintout) {
        char const* printout = nullptr;
        if (enabled) {
          printout = "enabled";
        } else {
          printout = "disabled";
        }
        FSFW_LOGI("TestDevice {} | getParameter: Changing datasets {}\n", deviceIdx, printout);
      }

      parameterWrapper->set(changingDatasets);
      break;
    }
    default:
      return INVALID_IDENTIFIER_ID;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

LocalPoolObjectBase* TestDevice::getPoolObjectHandle(lp_id_t localPoolId) {
  namespace td = testdevice;
  if (localPoolId == td::PoolIds::TEST_UINT8_ID) {
    return &dataset.testUint8Var;
  } else if (localPoolId == td::PoolIds::TEST_UINT32_ID) {
    return &dataset.testUint32Var;
  } else if (localPoolId == td::PoolIds::TEST_FLOAT_VEC_3_ID) {
    return &dataset.testFloat3Vec;
  } else {
    return nullptr;
  }
}
