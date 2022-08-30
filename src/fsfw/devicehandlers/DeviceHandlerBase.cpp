#include "fsfw/devicehandlers/DeviceHandlerBase.h"

#include "fsfw/datapoollocal/LocalPoolVariable.h"
#include "fsfw/devicehandlers/AcceptsDeviceResponsesIF.h"
#include "fsfw/devicehandlers/DeviceTmReportingWrapper.h"
#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/housekeeping/HousekeepingMessage.h"
#include "fsfw/ipc/MessageQueueMessage.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/subsystem/SubsystemBase.h"
#include "fsfw/thermal/ThermalComponentIF.h"

object_id_t DeviceHandlerBase::powerSwitcherId = objects::NO_OBJECT;
object_id_t DeviceHandlerBase::rawDataReceiverId = objects::NO_OBJECT;
object_id_t DeviceHandlerBase::defaultFdirParentId = objects::NO_OBJECT;

DeviceHandlerBase::DeviceHandlerBase(object_id_t setObjectId, object_id_t deviceCommunication,
                                     CookieIF* comCookie, FailureIsolationBase* fdirInstance,
                                     size_t cmdQueueSize)
    : SystemObject(setObjectId),
      mode(MODE_OFF),
      submode(SUBMODE_NONE),
      wiretappingMode(OFF),
      storedRawData(StorageManagerIF::INVALID_ADDRESS),
      deviceCommunicationId(deviceCommunication),
      comCookie(comCookie),
      healthHelper(this, setObjectId),
      modeHelper(this),
      parameterHelper(this),
      actionHelper(this, nullptr),
      poolManager(this, nullptr),
      childTransitionFailure(returnvalue::OK),
      fdirInstance(fdirInstance),
      defaultFDIRUsed(fdirInstance == nullptr),
      switchOffWasReported(false),
      childTransitionDelay(5000),
      transitionSourceMode(_MODE_POWER_DOWN),
      transitionSourceSubMode(SUBMODE_NONE) {
  commandQueue = QueueFactory::instance()->createMessageQueue(
      cmdQueueSize, MessageQueueMessage::MAX_MESSAGE_SIZE);
  insertInCommandMap(RAW_COMMAND_ID);
  cookieInfo.state = COOKIE_UNUSED;
  cookieInfo.pendingCommand = deviceCommandMap.end();
  if (comCookie == nullptr) {
    printWarningOrError(sif::OutputTypes::OUT_ERROR, "DeviceHandlerBase", returnvalue::FAILED,
                        "Invalid cookie");
  }
  if (this->fdirInstance == nullptr) {
    this->fdirInstance = new DeviceHandlerFailureIsolation(setObjectId, defaultFdirParentId);
  }
}

void DeviceHandlerBase::setHkDestination(object_id_t hkDestination) {
  this->hkDestination = hkDestination;
}

void DeviceHandlerBase::setThermalStateRequestPoolIds(lp_id_t thermalStatePoolId,
                                                      lp_id_t heaterRequestPoolId,
                                                      uint32_t thermalSetId) {
  thermalSet =
      new DeviceHandlerThermalSet(this, thermalSetId, thermalStatePoolId, heaterRequestPoolId);
}

DeviceHandlerBase::~DeviceHandlerBase() {
  if (comCookie != nullptr) {
    delete comCookie;
  }
  if (defaultFDIRUsed) {
    delete fdirInstance;
  }
  QueueFactory::instance()->deleteMessageQueue(commandQueue);
}

ReturnValue_t DeviceHandlerBase::performOperation(uint8_t counter) {
  this->pstStep = counter;
  this->lastStep = this->pstStep;

  if (getComAction() == CommunicationAction::NOTHING) {
    return returnvalue::OK;
  }

  if (getComAction() == CommunicationAction::PERFORM_OPERATION) {
    cookieInfo.state = COOKIE_UNUSED;
    readCommandQueue();
    doStateMachine();
    checkSwitchState();
    decrementDeviceReplyMap();
    fdirInstance->checkForFailures();
    performOperationHook();
    return returnvalue::OK;
  }

  if (mode == MODE_OFF) {
    return returnvalue::OK;
  }

  switch (getComAction()) {
    case CommunicationAction::SEND_WRITE:
      if (cookieInfo.state == COOKIE_UNUSED) {
        /* If no external command was specified, build internal command. */
        buildInternalCommand();
      }
      doSendWrite();
      break;
    case CommunicationAction::GET_WRITE:
      doGetWrite();
      break;
    case CommunicationAction::SEND_READ:
      doSendRead();
      break;
    case CommunicationAction::GET_READ:
      doGetRead();
      /* This will be performed after datasets have been updated by the
      custom device implementation. */
      poolManager.performHkOperation();
      break;
    default:
      break;
  }
  return returnvalue::OK;
}

ReturnValue_t DeviceHandlerBase::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != returnvalue::OK) {
    return result;
  }

  communicationInterface =
      ObjectManager::instance()->get<DeviceCommunicationIF>(deviceCommunicationId);
  if (communicationInterface == nullptr) {
    printWarningOrError(sif::OutputTypes::OUT_ERROR, "initialize",
                        ObjectManagerIF::CHILD_INIT_FAILED, "Passed communication IF invalid");
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  result = communicationInterface->initializeInterface(comCookie);
  if (result != returnvalue::OK) {
    printWarningOrError(sif::OutputTypes::OUT_ERROR, "initialize",
                        ObjectManagerIF::CHILD_INIT_FAILED, "ComIF initialization failed");
    return result;
  }

  IPCStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
  if (IPCStore == nullptr) {
    printWarningOrError(sif::OutputTypes::OUT_ERROR, "initialize",
                        ObjectManagerIF::CHILD_INIT_FAILED, "IPC Store not set up");
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  if (rawDataReceiverId != objects::NO_OBJECT) {
    AcceptsDeviceResponsesIF* rawReceiver =
        ObjectManager::instance()->get<AcceptsDeviceResponsesIF>(rawDataReceiverId);

    if (rawReceiver == nullptr) {
      printWarningOrError(sif::OutputTypes::OUT_ERROR, "initialize",
                          ObjectManagerIF::CHILD_INIT_FAILED,
                          "Raw receiver object ID set but no valid object found.");
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "Make sure the raw receiver object is set up properly"
                    " and implements AcceptsDeviceResponsesIF"
                 << std::endl;
#else
      sif::printError(
          "Make sure the raw receiver object is set up "
          "properly and implements AcceptsDeviceResponsesIF\n");
#endif
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
    defaultRawReceiver = rawReceiver->getDeviceQueue();
  }

  if (powerSwitcherId != objects::NO_OBJECT) {
    powerSwitcher = ObjectManager::instance()->get<PowerSwitchIF>(powerSwitcherId);
    if (powerSwitcher == nullptr) {
      printWarningOrError(sif::OutputTypes::OUT_ERROR, "initialize",
                          ObjectManagerIF::CHILD_INIT_FAILED,
                          "Power switcher set but no valid object found.");
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "Make sure the power switcher object is set up "
                 << "properly and implements PowerSwitchIF" << std::endl;
#else
      sif::printError(
          "Make sure the power switcher object is set up "
          "properly and implements PowerSwitchIF\n");
#endif
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  }

  result = healthHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }

  result = modeHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  result = actionHelper.initialize(commandQueue);
  if (result != returnvalue::OK) {
    return result;
  }
  result = fdirInstance->initialize();
  if (result != returnvalue::OK) {
    return result;
  }

  result = parameterHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }

  result = poolManager.initialize(commandQueue);
  if (result != returnvalue::OK) {
    return result;
  }

  fillCommandAndReplyMap();

  if (thermalSet != nullptr) {
    // Set temperature target state to NON_OP.
    result = thermalSet->read();
    if (result == returnvalue::OK) {
      thermalSet->heaterRequest.value = ThermalComponentIF::STATE_REQUEST_NON_OPERATIONAL;
      thermalSet->heaterRequest.setValid(true);
      thermalSet->commit();
    }
  }

  return returnvalue::OK;
}

void DeviceHandlerBase::decrementDeviceReplyMap() {
  bool timedOut = false;
  for (std::pair<const DeviceCommandId_t, DeviceReplyInfo>& replyPair : deviceReplyMap) {
    if (replyPair.second.countdown != nullptr && replyPair.second.active) {
      if (replyPair.second.countdown->hasTimedOut()) {
        resetTimeoutControlledReply(&replyPair.second);
        timedOut = true;
      }
    }
    if (replyPair.second.delayCycles != 0 && replyPair.second.countdown == nullptr) {
      replyPair.second.delayCycles--;
      if (replyPair.second.delayCycles == 0) {
        resetDelayCyclesControlledReply(&replyPair.second);
        timedOut = true;
      }
    }
    if (timedOut) {
      replyToReply(replyPair.first, replyPair.second, TIMEOUT);
      missedReply(replyPair.first);
      timedOut = false;
    }
  }
}

void DeviceHandlerBase::readCommandQueue() {
  if (dontCheckQueue()) {
    return;
  }

  CommandMessage command;
  ReturnValue_t result = commandQueue->receiveMessage(&command);
  if (result != returnvalue::OK) {
    return;
  }

  result = healthHelper.handleHealthCommand(&command);
  if (result == returnvalue::OK) {
    return;
  }

  result = modeHelper.handleModeCommand(&command);
  if (result == returnvalue::OK) {
    return;
  }

  result = actionHelper.handleActionMessage(&command);
  if (result == returnvalue::OK) {
    return;
  }

  result = parameterHelper.handleParameterMessage(&command);
  if (result == returnvalue::OK) {
    return;
  }

  result = poolManager.handleHousekeepingMessage(&command);
  if (result == returnvalue::OK) {
    return;
  }

  result = handleDeviceHandlerMessage(&command);
  if (result == returnvalue::OK) {
    return;
  }

  result = letChildHandleMessage(&command);
  if (result == returnvalue::OK) {
    return;
  }

  replyReturnvalueToCommand(CommandMessage::UNKNOWN_COMMAND);
}

void DeviceHandlerBase::doStateMachine() {
  switch (mode) {
    case _MODE_START_UP:
    case _MODE_SHUT_DOWN:
    case _MODE_TO_NORMAL:
    case _MODE_TO_ON:
    case _MODE_TO_RAW: {
      Mode_t currentMode = mode;
      callChildStatemachine();
      // Only do timeout if child did not change anything
      if (mode != currentMode) {
        break;
      }
      uint32_t currentUptime;
      Clock::getUptime(&currentUptime);
      if (currentUptime - timeoutStart >= childTransitionDelay) {
#if FSFW_VERBOSE_LEVEL >= 1 && FSFW_OBJ_EVENT_TRANSLATION == 0
        char printout[60];
        sprintf(printout, "Transition timeout (%lu) occured !",
                static_cast<unsigned long>(childTransitionDelay));
        /* Common configuration error for development, so print it */
        printWarningOrError(sif::OutputTypes::OUT_WARNING, "doStateMachine", returnvalue::FAILED,
                            printout);
#endif
        triggerEvent(MODE_TRANSITION_FAILED, childTransitionFailure, 0);
        setMode(transitionSourceMode, transitionSourceSubMode);
        break;
      }
    } break;
    case _MODE_POWER_DOWN:
      commandSwitch(PowerSwitchIF::SWITCH_OFF);
      setMode(_MODE_WAIT_OFF);
      break;
    case _MODE_POWER_ON:
      commandSwitch(PowerSwitchIF::SWITCH_ON);
      setMode(_MODE_WAIT_ON);
      break;
    case _MODE_WAIT_ON: {
      uint32_t currentUptime;
      Clock::getUptime(&currentUptime);
      if (powerSwitcher != nullptr and
          currentUptime - timeoutStart >= powerSwitcher->getSwitchDelayMs()) {
        triggerEvent(MODE_TRANSITION_FAILED, PowerSwitchIF::SWITCH_TIMEOUT, 0);
        setMode(_MODE_POWER_DOWN);
        callChildStatemachine();
        break;
      }
      ReturnValue_t switchState = getStateOfSwitches();
      if ((switchState == PowerSwitchIF::SWITCH_ON) || (switchState == NO_SWITCH)) {
        // NOTE: TransitionSourceMode and -SubMode are set by handleCommandedModeTransition
        childTransitionFailure = CHILD_TIMEOUT;
        setMode(_MODE_START_UP);
        callChildStatemachine();
      }
    } break;
    case _MODE_WAIT_OFF: {
      uint32_t currentUptime;
      Clock::getUptime(&currentUptime);

      if (powerSwitcher == nullptr) {
        setMode(MODE_OFF);
        break;
      }
      if (currentUptime - timeoutStart >= powerSwitcher->getSwitchDelayMs()) {
        triggerEvent(MODE_TRANSITION_FAILED, PowerSwitchIF::SWITCH_TIMEOUT, 0);
        setMode(MODE_ERROR_ON);
        break;
      }
      ReturnValue_t switchState = getStateOfSwitches();
      if ((switchState == PowerSwitchIF::SWITCH_OFF) || (switchState == NO_SWITCH)) {
        setMode(_MODE_SWITCH_IS_OFF);
      }
    } break;
    case MODE_OFF:
      doOffActivity();
      break;
    case MODE_ON:
      doOnActivity();
      break;
    case MODE_RAW:
    case MODE_NORMAL:
    case MODE_ERROR_ON:
      break;
    case _MODE_SWITCH_IS_OFF:
      setMode(MODE_OFF, SUBMODE_NONE);
      break;
    default:
      triggerEvent(OBJECT_IN_INVALID_MODE, mode, submode);
      setMode(_MODE_POWER_DOWN, 0);
      break;
  }
}

ReturnValue_t DeviceHandlerBase::isModeCombinationValid(Mode_t mode, Submode_t submode) {
  switch (mode) {
    case MODE_OFF:
    case MODE_ON:
    case MODE_NORMAL:
    case MODE_RAW:
      if (submode == SUBMODE_NONE) {
        return returnvalue::OK;
      } else {
        return INVALID_SUBMODE;
      }
    default:
      return HasModesIF::INVALID_MODE;
  }
}

ReturnValue_t DeviceHandlerBase::insertInCommandAndReplyMap(
    DeviceCommandId_t deviceCommand, uint16_t maxDelayCycles, LocalPoolDataSetBase* replyDataSet,
    size_t replyLen, bool periodic, bool hasDifferentReplyId, DeviceCommandId_t replyId,
    Countdown* countdown) {
  // No need to check, as we may try to insert multiple times.
  insertInCommandMap(deviceCommand, hasDifferentReplyId, replyId);
  if (hasDifferentReplyId) {
    return insertInReplyMap(replyId, maxDelayCycles, replyDataSet, replyLen, periodic, countdown);
  } else {
    return insertInReplyMap(deviceCommand, maxDelayCycles, replyDataSet, replyLen, periodic,
                            countdown);
  }
}

ReturnValue_t DeviceHandlerBase::insertInReplyMap(DeviceCommandId_t replyId,
                                                  uint16_t maxDelayCycles,
                                                  LocalPoolDataSetBase* dataSet, size_t replyLen,
                                                  bool periodic, Countdown* countdown) {
  DeviceReplyInfo info;
  info.maxDelayCycles = maxDelayCycles;
  info.periodic = periodic;
  info.delayCycles = 0;
  info.replyLen = replyLen;
  info.dataSet = dataSet;
  info.command = deviceCommandMap.end();
  info.countdown = countdown;
  auto resultPair = deviceReplyMap.emplace(replyId, info);
  if (resultPair.second) {
    return returnvalue::OK;
  } else {
    return returnvalue::FAILED;
  }
}

ReturnValue_t DeviceHandlerBase::insertInCommandMap(DeviceCommandId_t deviceCommand,
                                                    bool useAlternativeReply,
                                                    DeviceCommandId_t alternativeReplyId) {
  DeviceCommandInfo info;
  info.expectedReplies = 0;
  info.isExecuting = false;
  info.sendReplyTo = NO_COMMANDER;
  info.useAlternativeReplyId = alternativeReplyId;
  info.alternativeReplyId = alternativeReplyId;
  auto resultPair = deviceCommandMap.emplace(deviceCommand, info);
  if (resultPair.second) {
    return returnvalue::OK;
  } else {
    return returnvalue::FAILED;
  }
}

size_t DeviceHandlerBase::getNextReplyLength(DeviceCommandId_t commandId) {
  DeviceCommandId_t replyId = NO_COMMAND_ID;
  DeviceCommandMap::iterator command = cookieInfo.pendingCommand;
  if (command->second.useAlternativeReplyId) {
    replyId = command->second.alternativeReplyId;
  } else {
    replyId = commandId;
  }
  DeviceReplyIter iter = deviceReplyMap.find(replyId);
  if (iter != deviceReplyMap.end()) {
    if ((iter->second.delayCycles != 0 && iter->second.countdown == nullptr) ||
        (iter->second.active && iter->second.countdown != nullptr)) {
      return iter->second.replyLen;
    }
  }
  return 0;
}

ReturnValue_t DeviceHandlerBase::updateReplyMapEntry(DeviceCommandId_t deviceReply,
                                                     uint16_t delayCycles, uint16_t maxDelayCycles,
                                                     bool periodic) {
  auto replyIter = deviceReplyMap.find(deviceReply);
  if (replyIter == deviceReplyMap.end()) {
    triggerEvent(INVALID_DEVICE_COMMAND, deviceReply);
    return COMMAND_NOT_SUPPORTED;
  } else {
    DeviceReplyInfo* info = &(replyIter->second);
    if (maxDelayCycles != 0) {
      info->maxDelayCycles = maxDelayCycles;
    }
    info->delayCycles = delayCycles;
    info->periodic = periodic;
    return returnvalue::OK;
  }
}

ReturnValue_t DeviceHandlerBase::updatePeriodicReply(bool enable, DeviceCommandId_t deviceReply) {
  auto replyIter = deviceReplyMap.find(deviceReply);
  if (replyIter == deviceReplyMap.end()) {
    triggerEvent(INVALID_DEVICE_COMMAND, deviceReply);
    return COMMAND_NOT_SUPPORTED;
  } else {
    DeviceReplyInfo* info = &(replyIter->second);
    if (not info->periodic) {
      return COMMAND_NOT_SUPPORTED;
    }
    if (enable) {
      info->active = true;
      if (info->countdown != nullptr) {
        info->delayCycles = info->maxDelayCycles;
      } else {
        info->countdown->resetTimer();
      }
    } else {
      info->active = false;
      if (info->countdown != nullptr) {
        info->delayCycles = 0;
      } else {
        info->countdown->timeOut();
      }
    }
  }
  return returnvalue::OK;
}

ReturnValue_t DeviceHandlerBase::setReplyDataset(DeviceCommandId_t replyId,
                                                 LocalPoolDataSetBase* dataSet) {
  auto replyIter = deviceReplyMap.find(replyId);
  if (replyIter == deviceReplyMap.end()) {
    return returnvalue::FAILED;
  }
  replyIter->second.dataSet = dataSet;
  return returnvalue::OK;
}

void DeviceHandlerBase::callChildStatemachine() {
  if (mode == _MODE_START_UP) {
    doStartUp();
  } else if (mode == _MODE_SHUT_DOWN) {
    doShutDown();
  } else if (mode & TRANSITION_MODE_CHILD_ACTION_MASK) {
    doTransition(transitionSourceMode, transitionSourceSubMode);
  }
}

void DeviceHandlerBase::setTransition(Mode_t modeTo, Submode_t submodeTo) {
  triggerEvent(CHANGING_MODE, modeTo, submodeTo);
  childTransitionDelay = getTransitionDelayMs(mode, modeTo);
  transitionSourceMode = mode;
  transitionSourceSubMode = submode;
  childTransitionFailure = CHILD_TIMEOUT;

  // transitionTargetMode is set by setMode
  setMode((modeTo | TRANSITION_MODE_CHILD_ACTION_MASK), submodeTo);
}

void DeviceHandlerBase::setMode(Mode_t newMode, uint8_t newSubmode) {
  /* TODO: This will probably be done by the LocalDataPoolManager now */
  // changeHK(mode, submode, false);
  submode = newSubmode;
  mode = newMode;
  modeChanged();
  setNormalDatapoolEntriesInvalid();
  if (!isTransitionalMode()) {
    modeHelper.modeChanged(newMode, newSubmode);
    announceMode(false);
  }
  Clock::getUptime(&timeoutStart);

  if (mode == MODE_OFF and thermalSet != nullptr) {
    ReturnValue_t result = thermalSet->read();
    if (result == returnvalue::OK) {
      if (thermalSet->heaterRequest.value != ThermalComponentIF::STATE_REQUEST_IGNORE) {
        thermalSet->heaterRequest.value = ThermalComponentIF::STATE_REQUEST_NON_OPERATIONAL;
      }
      thermalSet->heaterRequest.commit(PoolVariableIF::VALID);
    }
  }
  /* TODO: This will probably be done by the LocalDataPoolManager now */
  // changeHK(mode, submode, true);
}

void DeviceHandlerBase::setMode(Mode_t newMode) { setMode(newMode, submode); }

void DeviceHandlerBase::replyReturnvalueToCommand(ReturnValue_t status, uint32_t parameter) {
  // This is actually the reply protocol for raw and misc DH commands.
  if (status == returnvalue::OK) {
    CommandMessage reply(CommandMessage::REPLY_COMMAND_OK, 0, parameter);
    commandQueue->reply(&reply);
  } else {
    CommandMessage reply(CommandMessage::REPLY_REJECTED, status, parameter);
    commandQueue->reply(&reply);
  }
}

void DeviceHandlerBase::replyToCommand(ReturnValue_t status, uint32_t parameter) {
  // Check if we reply to a raw command.
  if (cookieInfo.pendingCommand->first == RAW_COMMAND_ID) {
    if (status == NO_REPLY_EXPECTED) {
      status = returnvalue::OK;
    }
    replyReturnvalueToCommand(status, parameter);
    // Always delete data from a raw command.
    IPCStore->deleteData(storedRawData);
    return;
  }
  // Check if we were externally commanded.
  if (cookieInfo.pendingCommand->second.sendReplyTo != NO_COMMANDER) {
    MessageQueueId_t queueId = cookieInfo.pendingCommand->second.sendReplyTo;
    if (status == NO_REPLY_EXPECTED) {
      actionHelper.finish(true, queueId, cookieInfo.pendingCommand->first, returnvalue::OK);
    } else {
      actionHelper.step(1, queueId, cookieInfo.pendingCommand->first, status);
    }
  }
}

void DeviceHandlerBase::replyToReply(const DeviceCommandId_t command, DeviceReplyInfo& replyInfo,
                                     ReturnValue_t status) {
  // No need to check if iter exists, as this is checked by callers.
  // If someone else uses the method, add check.
  if (replyInfo.command == deviceCommandMap.end()) {
    // Is most likely periodic reply. Silent return.
    return;
  }
  DeviceCommandInfo* info = &replyInfo.command->second;
  if (info == nullptr) {
    printWarningOrError(sif::OutputTypes::OUT_ERROR, "replyToReply", returnvalue::FAILED,
                        "Command pointer not found");
    return;
  }

  if (info->expectedReplies > 0) {
    // Check before to avoid underflow
    info->expectedReplies--;
  }
  // Check if more replies are expected. If so, do nothing.
  if (info->expectedReplies == 0) {
    // Check if it was transition or internal command.
    // Don't send any replies in that case.
    if (info->sendReplyTo != NO_COMMANDER) {
      bool success = false;
      if (status == returnvalue::OK) {
        success = true;
      }
      actionHelper.finish(success, info->sendReplyTo, command, status);
    }
    info->isExecuting = false;
  }
}

void DeviceHandlerBase::doSendWrite() {
  if (cookieInfo.state == COOKIE_WRITE_READY) {
    ReturnValue_t result = communicationInterface->sendMessage(comCookie, rawPacket, rawPacketLen);

    if (result == returnvalue::OK) {
      cookieInfo.state = COOKIE_WRITE_SENT;
    } else {
      // always generate a failure event, so that FDIR knows what's up
      triggerEvent(DEVICE_SENDING_COMMAND_FAILED, result, cookieInfo.pendingCommand->first);
      replyToCommand(result);
      cookieInfo.state = COOKIE_UNUSED;
      cookieInfo.pendingCommand->second.isExecuting = false;
    }
  }
}

void DeviceHandlerBase::doGetWrite() {
  if (cookieInfo.state != COOKIE_WRITE_SENT) {
    return;
  }
  cookieInfo.state = COOKIE_UNUSED;
  ReturnValue_t result = communicationInterface->getSendSuccess(comCookie);
  if (result == returnvalue::OK) {
    if (wiretappingMode == RAW) {
      replyRawData(rawPacket, rawPacketLen, requestedRawTraffic, true);
    }

    // We need to distinguish here, because a raw command never expects a reply.
    //(Could be done in eRIRM, but then child implementations need to be careful.
    DeviceCommandMap::iterator command = cookieInfo.pendingCommand;
    result = enableReplyInReplyMap(command, 1, command->second.useAlternativeReplyId,
                                   command->second.alternativeReplyId);
  } else {
    // always generate a failure event, so that FDIR knows what's up
    triggerEvent(DEVICE_SENDING_COMMAND_FAILED, result, cookieInfo.pendingCommand->first);
  }
  if (result != returnvalue::OK) {
    cookieInfo.pendingCommand->second.isExecuting = false;
  }
  replyToCommand(result);
}

void DeviceHandlerBase::doSendRead() {
  ReturnValue_t result;

  result = doSendReadHook();
  if (result != returnvalue::OK) {
    return;
  }

  size_t replyLen = 0;
  if (cookieInfo.pendingCommand != deviceCommandMap.end()) {
    replyLen = getNextReplyLength(cookieInfo.pendingCommand->first);
  }

  result = communicationInterface->requestReceiveMessage(comCookie, replyLen);

  if (result == returnvalue::OK) {
    cookieInfo.state = COOKIE_READ_SENT;
  } else {
    triggerEvent(DEVICE_REQUESTING_REPLY_FAILED, result);
    // We can't inform anyone, because we don't know which command was sent last.
    // So, we need to wait for a timeout.
    // but I think we can allow to ignore one missedReply.
    ignoreMissedRepliesCount++;
    cookieInfo.state = COOKIE_UNUSED;
  }
}

void DeviceHandlerBase::doGetRead() {
  size_t receivedDataLen = 0;
  uint8_t* receivedData = nullptr;

  if (cookieInfo.state != COOKIE_READ_SENT) {
    cookieInfo.state = COOKIE_UNUSED;
    return;
  }

  cookieInfo.state = COOKIE_UNUSED;

  ReturnValue_t result =
      communicationInterface->readReceivedMessage(comCookie, &receivedData, &receivedDataLen);

  if (result != returnvalue::OK) {
    triggerEvent(DEVICE_REQUESTING_REPLY_FAILED, result);
    // I think we can allow to ignore one missedReply.
    ignoreMissedRepliesCount++;
    return;
  }

  if (receivedDataLen == 0 or result == DeviceCommunicationIF::NO_REPLY_RECEIVED) return;

  if (wiretappingMode == RAW) {
    replyRawData(receivedData, receivedDataLen, requestedRawTraffic);
  }

  if (mode == MODE_RAW) {
    if (defaultRawReceiver != MessageQueueIF::NO_QUEUE) {
      replyRawReplyIfnotWiretapped(receivedData, receivedDataLen);
    }
  } else {
    parseReply(receivedData, receivedDataLen);
  }
}

void DeviceHandlerBase::parseReply(const uint8_t* receivedData, size_t receivedDataLen) {
  ReturnValue_t result = returnvalue::FAILED;
  DeviceCommandId_t foundId = DeviceHandlerIF::NO_COMMAND_ID;
  size_t foundLen = 0;
  /* The loop may not execute more often than the number of received bytes
  (worst case). This approach avoids infinite loops due to buggy scanForReply routines. */
  uint32_t remainingLength = receivedDataLen;
  for (uint32_t count = 0; count < receivedDataLen; count++) {
    result = scanForReply(receivedData, remainingLength, &foundId, &foundLen);
    switch (result) {
      case returnvalue::OK:
        handleReply(receivedData, foundId, foundLen);
        if (foundLen == 0) {
          printWarningOrError(sif::OutputTypes::OUT_WARNING, "parseReply",
                              ObjectManagerIF::CHILD_INIT_FAILED,
                              "Found length is one, parsing might be stuck");
        }
        break;
      case APERIODIC_REPLY: {
        result = interpretDeviceReply(foundId, receivedData);
        if (result != returnvalue::OK) {
          replyRawReplyIfnotWiretapped(receivedData, foundLen);
          triggerEvent(DEVICE_INTERPRETING_REPLY_FAILED, result, foundId);
        }
        if (foundLen == 0) {
          printWarningOrError(sif::OutputTypes::OUT_ERROR, "parseReply",
                              ObjectManagerIF::CHILD_INIT_FAILED,
                              "Power switcher set but no valid object found.");
#if FSFW_CPP_OSTREAM_ENABLED == 1
          sif::warning << "DeviceHandlerBase::parseReply: foundLen is 0!"
                          " Packet parsing will be stuck."
                       << std::endl;
#endif
        }
        break;
      }
      case IGNORE_REPLY_DATA:
        continue;
      case IGNORE_FULL_PACKET:
        return;
      default:
        // We need to wait for timeout.. don't know what command failed
        // and who sent it.
        replyRawReplyIfnotWiretapped(receivedData, foundLen);
        triggerEvent(DEVICE_READING_REPLY_FAILED, result, foundLen);
        break;
    }
    receivedData += foundLen;
    if (remainingLength > foundLen) {
      remainingLength -= foundLen;
    } else {
      return;
    }
  }
}

void DeviceHandlerBase::handleReply(const uint8_t* receivedData, DeviceCommandId_t foundId,
                                    uint32_t foundLen) {
  ReturnValue_t result;
  DeviceReplyMap::iterator iter = deviceReplyMap.find(foundId);

  if (iter == deviceReplyMap.end()) {
    replyRawReplyIfnotWiretapped(receivedData, foundLen);
    triggerEvent(DEVICE_UNKNOWN_REPLY, foundId);
    return;
  }

  DeviceReplyInfo* info = &(iter->second);

  if ((info->delayCycles != 0 && info->countdown == nullptr) ||
      (info->active && info->countdown != nullptr)) {
    result = interpretDeviceReply(foundId, receivedData);

    if (result == IGNORE_REPLY_DATA) {
      return;
    }

    if (info->active && info->countdown != nullptr) {
      resetTimeoutControlledReply(info);
    } else if (info->delayCycles != 0) {
      resetDelayCyclesControlledReply(info);
    }

    if (result != returnvalue::OK) {
      // Report failed interpretation to FDIR.
      replyRawReplyIfnotWiretapped(receivedData, foundLen);
      triggerEvent(DEVICE_INTERPRETING_REPLY_FAILED, result, foundId);
    }
    replyToReply(iter->first, iter->second, result);
  } else {
    /* Other completion failure messages are created by timeout.
    Powering down the device might take some time during which periodic
    replies may still come in. */
    if (mode != _MODE_WAIT_OFF) {
      triggerEvent(DEVICE_UNREQUESTED_REPLY, foundId);
    }
  }
}

void DeviceHandlerBase::resetTimeoutControlledReply(DeviceReplyInfo* info) {
  if (info->periodic) {
    info->countdown->resetTimer();
  } else {
    info->active = false;
    info->countdown->timeOut();
  }
}

void DeviceHandlerBase::resetDelayCyclesControlledReply(DeviceReplyInfo* info) {
  if (info->periodic) {
    info->delayCycles = info->maxDelayCycles;
  } else {
    info->delayCycles = 0;
    info->active = false;
  }
}

ReturnValue_t DeviceHandlerBase::getStorageData(store_address_t storageAddress, uint8_t** data,
                                                size_t* len) {
  size_t lenTmp;

  if (IPCStore == nullptr) {
    *data = nullptr;
    *len = 0;
    return returnvalue::FAILED;
  }
  ReturnValue_t result = IPCStore->modifyData(storageAddress, data, &lenTmp);
  if (result == returnvalue::OK) {
    *len = lenTmp;
    return returnvalue::OK;
  } else {
    triggerEvent(StorageManagerIF::GET_DATA_FAILED, result, storageAddress.raw);
    *data = nullptr;
    *len = 0;
    return result;
  }
}

void DeviceHandlerBase::replyRawData(const uint8_t* data, size_t len, MessageQueueId_t sendTo,
                                     bool isCommand) {
  if (IPCStore == nullptr or len == 0 or sendTo == MessageQueueIF::NO_QUEUE) {
    return;
  }
  store_address_t address;
  ReturnValue_t result = IPCStore->addData(&address, data, len);

  if (result != returnvalue::OK) {
    triggerEvent(StorageManagerIF::STORE_DATA_FAILED, result);
    return;
  }

  CommandMessage command;

  DeviceHandlerMessage::setDeviceHandlerRawReplyMessage(&command, getObjectId(), address,
                                                        isCommand);

  result = commandQueue->sendMessage(sendTo, &command);

  if (result != returnvalue::OK) {
    IPCStore->deleteData(address);
    // Silently discard data, this indicates heavy TM traffic which
    // should not be increased by additional events.
  }
}

// Default child implementations
DeviceHandlerIF::CommunicationAction DeviceHandlerBase::getComAction() {
  switch (pstStep) {
    case 0:
      return CommunicationAction::PERFORM_OPERATION;
      break;
    case 1:
      return CommunicationAction::SEND_WRITE;
      break;
    case 2:
      return CommunicationAction::GET_WRITE;
      break;
    case 3:
      return CommunicationAction::SEND_READ;
      break;
    case 4:
      return CommunicationAction::GET_READ;
      break;
    default:
      break;
  }
  return CommunicationAction::NOTHING;
}

MessageQueueId_t DeviceHandlerBase::getCommandQueue() const { return commandQueue->getId(); }

void DeviceHandlerBase::buildRawDeviceCommand(CommandMessage* commandMessage) {
  storedRawData = DeviceHandlerMessage::getStoreAddress(commandMessage);
  ReturnValue_t result = getStorageData(storedRawData, &rawPacket, &rawPacketLen);
  if (result != returnvalue::OK) {
    replyReturnvalueToCommand(result, RAW_COMMAND_ID);
    storedRawData.raw = StorageManagerIF::INVALID_ADDRESS;
  } else {
    cookieInfo.pendingCommand = deviceCommandMap.find((DeviceCommandId_t)RAW_COMMAND_ID);
    cookieInfo.pendingCommand->second.isExecuting = true;
    cookieInfo.state = COOKIE_WRITE_READY;
  }
}

void DeviceHandlerBase::commandSwitch(ReturnValue_t onOff) {
  if (powerSwitcher == nullptr) {
    return;
  }
  const uint8_t* switches;
  uint8_t numberOfSwitches = 0;
  ReturnValue_t result = getSwitches(&switches, &numberOfSwitches);
  if (result == returnvalue::OK) {
    while (numberOfSwitches > 0) {
      powerSwitcher->sendSwitchCommand(switches[numberOfSwitches - 1], onOff);
      numberOfSwitches--;
    }
  }
}

ReturnValue_t DeviceHandlerBase::doSendReadHook() { return returnvalue::OK; }

ReturnValue_t DeviceHandlerBase::getSwitches(const uint8_t** switches, uint8_t* numberOfSwitches) {
  return DeviceHandlerBase::NO_SWITCH;
}

void DeviceHandlerBase::modeChanged(void) {}

ReturnValue_t DeviceHandlerBase::enableReplyInReplyMap(DeviceCommandMap::iterator command,
                                                       uint8_t expectedReplies,
                                                       bool useAlternativeId,
                                                       DeviceCommandId_t alternativeReply) {
  DeviceReplyMap::iterator iter;
  if (useAlternativeId) {
    iter = deviceReplyMap.find(alternativeReply);
  } else {
    iter = deviceReplyMap.find(command->first);
  }
  if (iter != deviceReplyMap.end()) {
    DeviceReplyInfo* info = &(iter->second);
    // If a countdown has been set, the delay cycles will be ignored and the reply times out
    // as soon as the countdown has expired
    info->delayCycles = info->maxDelayCycles;
    info->command = command;
    command->second.expectedReplies = expectedReplies;
    if (info->countdown != nullptr) {
      info->countdown->resetTimer();
    }
    info->active = true;
    return returnvalue::OK;
  } else {
    return NO_REPLY_EXPECTED;
  }
}

void DeviceHandlerBase::doTransition(Mode_t modeFrom, Submode_t subModeFrom) {
  setMode(getBaseMode(mode));
}

ReturnValue_t DeviceHandlerBase::getStateOfSwitches(void) {
  if (powerSwitcher == nullptr) {
    return NO_SWITCH;
  }
  uint8_t numberOfSwitches = 0;
  const uint8_t* switches;

  ReturnValue_t result = getSwitches(&switches, &numberOfSwitches);
  if ((result == returnvalue::OK) && (numberOfSwitches != 0)) {
    while (numberOfSwitches > 0) {
      if (powerSwitcher->getSwitchState(switches[numberOfSwitches - 1]) ==
          PowerSwitchIF::SWITCH_OFF) {
        return PowerSwitchIF::SWITCH_OFF;
      }
      numberOfSwitches--;
    }
    return PowerSwitchIF::SWITCH_ON;
  }

  return NO_SWITCH;
}

Mode_t DeviceHandlerBase::getBaseMode(Mode_t transitionMode) {
  // only child action special modes are handled, as a child should
  // never see any base action modes
  if (transitionMode == _MODE_START_UP) {
    return _MODE_TO_ON;
  }
  if (transitionMode == _MODE_SHUT_DOWN) {
    return _MODE_POWER_DOWN;
  }
  return transitionMode & ~(TRANSITION_MODE_BASE_ACTION_MASK | TRANSITION_MODE_CHILD_ACTION_MASK);
}

// SHOULDDO: Allow transition from OFF to NORMAL to reduce complexity in assemblies. And, by the
// way, throw away DHB and write a new one:
//  - Include power and thermal completely, but more modular :-)
//  - Don't use modes for state transitions, reduce FSM (Finte State Machine) complexity.
//  - Modularization?
ReturnValue_t DeviceHandlerBase::checkModeCommand(Mode_t commandedMode, Submode_t commandedSubmode,
                                                  uint32_t* msToReachTheMode) {
  if (isTransitionalMode()) {
    return IN_TRANSITION;
  }
  if ((mode == MODE_ERROR_ON) && (commandedMode != MODE_OFF)) {
    return TRANS_NOT_ALLOWED;
  }
  if ((commandedMode == MODE_NORMAL) && (mode == MODE_OFF)) {
    return TRANS_NOT_ALLOWED;
  }

  if ((commandedMode == MODE_ON) && (mode == MODE_OFF) and (thermalSet != nullptr)) {
    ReturnValue_t result = thermalSet->read();
    if (result == returnvalue::OK) {
      if ((thermalSet->heaterRequest.value != ThermalComponentIF::STATE_REQUEST_IGNORE) and
          (not ThermalComponentIF::isOperational(thermalSet->thermalState.value))) {
        triggerEvent(ThermalComponentIF::TEMP_NOT_IN_OP_RANGE, thermalSet->thermalState.value);
        return NON_OP_TEMPERATURE;
      }
    }
  }

  return isModeCombinationValid(commandedMode, commandedSubmode);
}

void DeviceHandlerBase::startTransition(Mode_t commandedMode, Submode_t commandedSubmode) {
  switch (commandedMode) {
    case MODE_ON:
      handleTransitionToOnMode(commandedMode, commandedSubmode);
      break;
    case MODE_OFF:
      if (mode == MODE_OFF) {
        triggerEvent(CHANGING_MODE, commandedMode, commandedSubmode);
        setMode(_MODE_POWER_DOWN, commandedSubmode);
      } else {
        // already set the delay for the child transition
        // so we don't need to call it twice
        childTransitionDelay = getTransitionDelayMs(mode, _MODE_POWER_DOWN);
        transitionSourceMode = _MODE_POWER_DOWN;
        transitionSourceSubMode = commandedSubmode;
        childTransitionFailure = CHILD_TIMEOUT;
        setMode(_MODE_SHUT_DOWN, commandedSubmode);
        triggerEvent(CHANGING_MODE, commandedMode, commandedSubmode);
      }
      break;
    case MODE_RAW:
      if (mode != MODE_OFF) {
        setTransition(MODE_RAW, commandedSubmode);
      } else {
        setMode(MODE_RAW, commandedSubmode);
      }
      break;
    case MODE_NORMAL:
      if (mode != MODE_OFF) {
        setTransition(MODE_NORMAL, commandedSubmode);
      } else {
        replyReturnvalueToCommand(HasModesIF::TRANS_NOT_ALLOWED);
      }
      break;
  }
}

void DeviceHandlerBase::handleTransitionToOnMode(Mode_t commandedMode, Submode_t commandedSubmode) {
  if (mode == MODE_OFF) {
    transitionSourceMode = _MODE_POWER_DOWN;
    transitionSourceSubMode = SUBMODE_NONE;
    setMode(_MODE_POWER_ON, commandedSubmode);
    // already set the delay for the child transition so we don't
    // need to call it twice
    childTransitionDelay = getTransitionDelayMs(_MODE_START_UP, MODE_ON);
    triggerEvent(CHANGING_MODE, commandedMode, commandedSubmode);
    if (thermalSet != nullptr) {
      ReturnValue_t result = thermalSet->read();
      if (result == returnvalue::OK) {
        if (thermalSet->heaterRequest != ThermalComponentIF::STATE_REQUEST_IGNORE) {
          thermalSet->heaterRequest = ThermalComponentIF::STATE_REQUEST_OPERATIONAL;
          thermalSet->commit();
        }
      }
    }
  } else {
    setTransition(MODE_ON, commandedSubmode);
  }
}

void DeviceHandlerBase::getMode(Mode_t* mode, Submode_t* submode) {
  *mode = this->mode;
  *submode = this->submode;
}

void DeviceHandlerBase::setToExternalControl() { healthHelper.setHealth(EXTERNAL_CONTROL); }

void DeviceHandlerBase::announceMode(bool recursive) { triggerEvent(MODE_INFO, mode, submode); }

bool DeviceHandlerBase::dontCheckQueue() { return false; }

void DeviceHandlerBase::missedReply(DeviceCommandId_t id) {
  if (ignoreMissedRepliesCount > 0) {
    ignoreMissedRepliesCount--;
  } else {
    triggerEvent(DEVICE_MISSED_REPLY, id);
  }
}

HasHealthIF::HealthState DeviceHandlerBase::getHealth() { return healthHelper.getHealth(); }

ReturnValue_t DeviceHandlerBase::setHealth(HealthState health) {
  healthHelper.setHealth(health);
  return returnvalue::OK;
}

void DeviceHandlerBase::checkSwitchState() {
  if ((mode == MODE_ON || mode == MODE_NORMAL)) {
    // We only check in ON and NORMAL, ignore RAW and ERROR_ON.
    ReturnValue_t result = getStateOfSwitches();
    if (result == PowerSwitchIF::SWITCH_OFF && !switchOffWasReported) {
      triggerEvent(PowerSwitchIF::SWITCH_WENT_OFF);
      switchOffWasReported = true;
    }
  } else {
    switchOffWasReported = false;
  }
}

void DeviceHandlerBase::doOnActivity() {}

ReturnValue_t DeviceHandlerBase::acceptExternalDeviceCommands() {
  if ((mode != MODE_ON) && (mode != MODE_NORMAL)) {
    return WRONG_MODE_FOR_COMMAND;
  }
  return returnvalue::OK;
}

void DeviceHandlerBase::replyRawReplyIfnotWiretapped(const uint8_t* data, size_t len) {
  if ((wiretappingMode == RAW) && (defaultRawReceiver == requestedRawTraffic)) {
    // The raw packet was already sent by the wiretapping service
  } else {
    replyRawData(data, len, defaultRawReceiver);
  }
}

ReturnValue_t DeviceHandlerBase::handleDeviceHandlerMessage(CommandMessage* message) {
  switch (message->getCommand()) {
    case DeviceHandlerMessage::CMD_WIRETAPPING:
      switch (DeviceHandlerMessage::getWiretappingMode(message)) {
        case RAW:
          wiretappingMode = RAW;
          requestedRawTraffic = commandQueue->getLastPartner();
          break;
        case TM:
          wiretappingMode = TM;
          requestedRawTraffic = commandQueue->getLastPartner();
          break;
        case OFF:
          wiretappingMode = OFF;
          break;
        default:
          replyReturnvalueToCommand(INVALID_COMMAND_PARAMETER);
          wiretappingMode = OFF;
          return returnvalue::OK;
      }
      replyReturnvalueToCommand(returnvalue::OK);
      return returnvalue::OK;
    case DeviceHandlerMessage::CMD_RAW:
      if ((mode != MODE_RAW)) {
        DeviceHandlerMessage::clear(message);
        replyReturnvalueToCommand(WRONG_MODE_FOR_COMMAND);
      } else {
        buildRawDeviceCommand(message);
      }
      return returnvalue::OK;
    default:
      return returnvalue::FAILED;
  }
}

void DeviceHandlerBase::setParentQueue(MessageQueueId_t parentQueueId) {
  modeHelper.setParentQueue(parentQueueId);
  healthHelper.setParentQueue(parentQueueId);
}

bool DeviceHandlerBase::isAwaitingReply() {
  std::map<DeviceCommandId_t, DeviceReplyInfo>::iterator iter;
  for (iter = deviceReplyMap.begin(); iter != deviceReplyMap.end(); ++iter) {
    if ((iter->second.delayCycles != 0 && iter->second.countdown == nullptr) ||
        (iter->second.active && iter->second.countdown != nullptr)) {
      return true;
    }
  }
  return false;
}

ReturnValue_t DeviceHandlerBase::letChildHandleMessage(CommandMessage* message) {
  return returnvalue::FAILED;
}

void DeviceHandlerBase::handleDeviceTm(util::DataWrapper dataWrapper, DeviceCommandId_t replyId,
                                       bool forceDirectTm) {
  if (dataWrapper.isNull()) {
    return;
  }

  auto iter = deviceReplyMap.find(replyId);
  if (iter == deviceReplyMap.end()) {
    triggerEvent(DEVICE_UNKNOWN_REPLY, replyId);
    return;
  }

  auto reportData = [&](MessageQueueId_t queueId) {
    if (dataWrapper.type == util::DataTypes::SERIALIZABLE) {
      return actionHelper.reportData(queueId, replyId, dataWrapper.dataUnion.serializable);
    } else if (dataWrapper.type == util::DataTypes::RAW) {
      return actionHelper.reportData(queueId, replyId, dataWrapper.dataUnion.raw.data,
                                     dataWrapper.dataUnion.raw.len);
    }
    return returnvalue::FAILED;
  };

  // Regular replies to a command
  if (iter->second.command != deviceCommandMap.end()) {
    MessageQueueId_t queueId = iter->second.command->second.sendReplyTo;

    // This may fail, but we'll ignore the fault.
    if (queueId != NO_COMMANDER) {
      reportData(queueId);
    }

    // This check should make sure we get any TM but don't get anything doubled.
    if (wiretappingMode == TM && (requestedRawTraffic != queueId)) {
      DeviceTmReportingWrapper wrapper(getObjectId(), replyId, dataWrapper);
      actionHelper.reportData(requestedRawTraffic, replyId, &wrapper);
    }

    else if (forceDirectTm and (defaultRawReceiver != queueId) and
             (defaultRawReceiver != MessageQueueIF::NO_QUEUE)) {
      // hiding of sender needed so the service will handle it as
      // unexpected Data, no matter what state (progress or completed)
      // it is in
      reportData(defaultRawReceiver);
    }
  }
  // Unrequested or aperiodic replies
  else {
    DeviceTmReportingWrapper wrapper(getObjectId(), replyId, dataWrapper);
    if (wiretappingMode == TM) {
      actionHelper.reportData(requestedRawTraffic, replyId, &wrapper);
    }
    if (forceDirectTm and defaultRawReceiver != MessageQueueIF::NO_QUEUE) {
      // hiding of sender needed so the service will handle it as
      // unexpected Data, no matter what state (progress or completed)
      // it is in
      actionHelper.reportData(defaultRawReceiver, replyId, &wrapper, true);
    }
  }
}

ReturnValue_t DeviceHandlerBase::executeAction(ActionId_t actionId, MessageQueueId_t commandedBy,
                                               const uint8_t* data, size_t size) {
  ReturnValue_t result = acceptExternalDeviceCommands();
  if (result != returnvalue::OK) {
    return result;
  }
  DeviceCommandMap::iterator iter = deviceCommandMap.find(actionId);
  if (iter == deviceCommandMap.end()) {
    result = COMMAND_NOT_SUPPORTED;
  } else if (iter->second.isExecuting) {
    result = COMMAND_ALREADY_SENT;
  } else {
    result = buildCommandFromCommand(actionId, data, size);
  }
  if (result == returnvalue::OK) {
    iter->second.sendReplyTo = commandedBy;
    iter->second.isExecuting = true;
    cookieInfo.pendingCommand = iter;
    cookieInfo.state = COOKIE_WRITE_READY;
  }
  return result;
}

void DeviceHandlerBase::buildInternalCommand(void) {
  /* Neither raw nor direct could build a command */
  ReturnValue_t result = NOTHING_TO_SEND;
  DeviceCommandId_t deviceCommandId = NO_COMMAND_ID;
  if (mode == MODE_NORMAL) {
    result = buildNormalDeviceCommand(&deviceCommandId);
    if (result == BUSY) {
      /* So we can track misconfigurations */
      printWarningOrError(sif::OutputTypes::OUT_WARNING, "buildInternalCommand",
                          returnvalue::FAILED, "Busy.");
      /* No need to report this */
      result = NOTHING_TO_SEND;
    }
  } else if (mode == MODE_RAW) {
    result = buildChildRawCommand();
    deviceCommandId = RAW_COMMAND_ID;
  } else if (mode & TRANSITION_MODE_CHILD_ACTION_MASK) {
    result = buildTransitionDeviceCommand(&deviceCommandId);
  } else {
    return;
  }

  if (result == NOTHING_TO_SEND) {
    return;
  }
  if (result == returnvalue::OK) {
    DeviceCommandMap::iterator iter = deviceCommandMap.find(deviceCommandId);
    if (iter == deviceCommandMap.end()) {
#if FSFW_VERBOSE_LEVEL >= 1
      char output[36];
      sprintf(output, "Command 0x%08x unknown", static_cast<unsigned int>(deviceCommandId));
      // so we can track misconfigurations
      printWarningOrError(sif::OutputTypes::OUT_WARNING, "buildInternalCommand",
                          COMMAND_NOT_SUPPORTED, output);
#endif
      result = COMMAND_NOT_SUPPORTED;
    } else if (iter->second.isExecuting) {
#if FSFW_VERBOSE_LEVEL >= 1
      char output[36];
      sprintf(output, "Command 0x%08x is executing", static_cast<unsigned int>(deviceCommandId));
      // so we can track misconfigurations
      printWarningOrError(sif::OutputTypes::OUT_WARNING, "buildInternalCommand",
                          returnvalue::FAILED, output);
#endif
      // this is an internal command, no need to report a failure here,
      // missed reply will track if a reply is too late, otherwise, it's ok
      return;
    } else {
      iter->second.sendReplyTo = NO_COMMANDER;
      iter->second.isExecuting = true;
      cookieInfo.pendingCommand = iter;
      cookieInfo.state = COOKIE_WRITE_READY;
    }
  }
  if (result != returnvalue::OK) {
    triggerEvent(DEVICE_BUILDING_COMMAND_FAILED, result, deviceCommandId);
  }
}

ReturnValue_t DeviceHandlerBase::buildChildRawCommand() { return NOTHING_TO_SEND; }

uint8_t DeviceHandlerBase::getReplyDelayCycles(DeviceCommandId_t deviceCommand) {
  DeviceReplyMap::iterator iter = deviceReplyMap.find(deviceCommand);
  if (iter == deviceReplyMap.end()) {
    return 0;
  } else if (iter->second.countdown != nullptr) {
    // fake a useful return value for legacy code
    if (iter->second.active) {
      return 1;
    } else {
      return 0;
    }
  }
  return iter->second.delayCycles;
}

Mode_t DeviceHandlerBase::getTransitionSourceMode() const { return transitionSourceMode; }

Submode_t DeviceHandlerBase::getTransitionSourceSubMode() const { return transitionSourceSubMode; }

void DeviceHandlerBase::triggerEvent(Event event, uint32_t parameter1, uint32_t parameter2) {
  fdirInstance->triggerEvent(event, parameter1, parameter2);
}

void DeviceHandlerBase::forwardEvent(Event event, uint32_t parameter1, uint32_t parameter2) const {
  fdirInstance->triggerEvent(event, parameter1, parameter2);
}

void DeviceHandlerBase::doOffActivity() {}

ReturnValue_t DeviceHandlerBase::getParameter(uint8_t domainId, uint8_t uniqueId,
                                              ParameterWrapper* parameterWrapper,
                                              const ParameterWrapper* newValues,
                                              uint16_t startAtIndex) {
  ReturnValue_t result =
      fdirInstance->getParameter(domainId, uniqueId, parameterWrapper, newValues, startAtIndex);
  if (result != INVALID_DOMAIN_ID) {
    return result;
  }
  return INVALID_DOMAIN_ID;
}

bool DeviceHandlerBase::isTransitionalMode() {
  return ((mode & (TRANSITION_MODE_BASE_ACTION_MASK | TRANSITION_MODE_CHILD_ACTION_MASK)) != 0);
}

bool DeviceHandlerBase::commandIsExecuting(DeviceCommandId_t commandId) {
  auto iter = deviceCommandMap.find(commandId);
  if (iter != deviceCommandMap.end()) {
    return iter->second.isExecuting;
  } else {
    return false;
  }
}

void DeviceHandlerBase::setTaskIF(PeriodicTaskIF* task) { executingTask = task; }

void DeviceHandlerBase::debugInterface(uint8_t positionTracker, object_id_t objectId,
                                       uint32_t parameter) {}

void DeviceHandlerBase::performOperationHook() {}

ReturnValue_t DeviceHandlerBase::initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
                                                         LocalDataPoolManager& poolManager) {
  if (thermalSet != nullptr) {
    localDataPoolMap.emplace(thermalSet->thermalStatePoolId,
                             new PoolEntry<DeviceHandlerIF::dh_thermal_state_t>);
    localDataPoolMap.emplace(thermalSet->heaterRequestPoolId,
                             new PoolEntry<DeviceHandlerIF::dh_heater_request_t>);
  }
  return returnvalue::OK;
}

ReturnValue_t DeviceHandlerBase::initializeAfterTaskCreation() {
  // In this function, the task handle should be valid if the task
  // was implemented correctly. We still check to be 1000 % sure :-)
  if (executingTask != nullptr) {
    pstIntervalMs = executingTask->getPeriodMs();
  }
  this->poolManager.initializeAfterTaskCreation();

  if (setStartupImmediately) {
    startTransition(MODE_ON, SUBMODE_NONE);
  }
  return returnvalue::OK;
}

LocalPoolDataSetBase* DeviceHandlerBase::getDataSetHandle(sid_t sid) {
  auto iter = deviceReplyMap.find(sid.ownerSetId);
  if (iter != deviceReplyMap.end()) {
    return iter->second.dataSet;
  } else {
    return nullptr;
  }
}

object_id_t DeviceHandlerBase::getObjectId() const { return SystemObject::getObjectId(); }

void DeviceHandlerBase::setStartUpImmediately() { this->setStartupImmediately = true; }

dur_millis_t DeviceHandlerBase::getPeriodicOperationFrequency() const { return pstIntervalMs; }

DeviceCommandId_t DeviceHandlerBase::getPendingCommand() const {
  if (cookieInfo.pendingCommand != deviceCommandMap.end()) {
    return cookieInfo.pendingCommand->first;
  }
  return DeviceHandlerIF::NO_COMMAND_ID;
}

void DeviceHandlerBase::setNormalDatapoolEntriesInvalid() {
  for (const auto& reply : deviceReplyMap) {
    if (reply.second.dataSet != nullptr) {
      reply.second.dataSet->setValidity(false, true);
    }
  }
}

void DeviceHandlerBase::printWarningOrError(sif::OutputTypes errorType, const char* functionName,
                                            ReturnValue_t errorCode, const char* errorPrint) {
  if (errorPrint == nullptr) {
    if (errorCode == ObjectManagerIF::CHILD_INIT_FAILED) {
      errorPrint = "Initialization error";
    } else if (errorCode == returnvalue::FAILED) {
      if (errorType == sif::OutputTypes::OUT_WARNING) {
        errorPrint = "Generic Warning";
      } else {
        errorPrint = "Generic Error";
      }
    } else {
      errorPrint = "Unknown error";
    }
  }
  if (functionName == nullptr) {
    functionName = "unknown function";
  }

  if (errorType == sif::OutputTypes::OUT_WARNING) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "DeviceHandlerBase::" << functionName << ": Object ID 0x" << std::hex
                 << std::setw(8) << std::setfill('0') << this->getObjectId() << " | " << errorPrint
                 << std::dec << std::setfill(' ') << std::endl;
#else
    sif::printWarning("DeviceHandlerBase::%s: Object ID 0x%08x | %s\n", functionName,
                      this->getObjectId(), errorPrint);
#endif
  } else if (errorType == sif::OutputTypes::OUT_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "DeviceHandlerBase::" << functionName << ": Object ID 0x" << std::hex
               << std::setw(8) << std::setfill('0') << this->getObjectId() << " | " << errorPrint
               << std::dec << std::setfill(' ') << std::endl;
#else
    sif::printError("DeviceHandlerBase::%s: Object ID 0x%08x | %s\n", functionName,
                    this->getObjectId(), errorPrint);
#endif
  }
}

LocalDataPoolManager* DeviceHandlerBase::getHkManagerHandle() { return &poolManager; }

MessageQueueId_t DeviceHandlerBase::getCommanderQueueId(DeviceCommandId_t replyId) const {
  auto commandIter = deviceCommandMap.find(replyId);
  if (commandIter == deviceCommandMap.end()) {
    return MessageQueueIF::NO_QUEUE;
  }
  return commandIter->second.sendReplyTo;
}
