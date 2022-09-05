#include "fsfw/tmtcservices/CommandingServiceBase.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tcdistribution/PusDistributorIF.h"
#include "fsfw/tmtcpacket/pus/tc.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"
#include "fsfw/tmtcservices/TmTcMessage.h"
#include "fsfw/tmtcservices/tcHelpers.h"
#include "fsfw/tmtcservices/tmHelpers.h"

object_id_t CommandingServiceBase::defaultPacketSource = objects::NO_OBJECT;
object_id_t CommandingServiceBase::defaultPacketDestination = objects::NO_OBJECT;

CommandingServiceBase::CommandingServiceBase(object_id_t setObjectId, uint16_t apid,
                                             const char* name, uint8_t service,
                                             uint8_t numberOfParallelCommands,
                                             uint16_t commandTimeoutSeconds, size_t queueDepth,
                                             VerificationReporterIF* verificationReporter)
    : SystemObject(setObjectId),
      apid(apid),
      service(service),
      timeoutSeconds(commandTimeoutSeconds),
      tmStoreHelper(apid),
      tmHelper(service, tmStoreHelper, tmSendHelper),
      verificationReporter(verificationReporter),
      commandMap(numberOfParallelCommands),
      name(name) {
  commandQueue = QueueFactory::instance()->createMessageQueue(queueDepth);
  requestQueue = QueueFactory::instance()->createMessageQueue(queueDepth);
}

void CommandingServiceBase::setPacketSource(object_id_t packetSource_) {
  packetSource = packetSource_;
}

void CommandingServiceBase::setPacketDestination(object_id_t packetDestination_) {
  packetDestination = packetDestination_;
}

CommandingServiceBase::~CommandingServiceBase() {
  QueueFactory::instance()->deleteMessageQueue(commandQueue);
  QueueFactory::instance()->deleteMessageQueue(requestQueue);
}

ReturnValue_t CommandingServiceBase::performOperation(uint8_t opCode) {
  handleCommandQueue();
  handleRequestQueue();
  checkTimeout();
  doPeriodicOperation();
  return returnvalue::OK;
}

uint32_t CommandingServiceBase::getIdentifier() const { return service; }

MessageQueueId_t CommandingServiceBase::getRequestQueue() const { return requestQueue->getId(); }

ReturnValue_t CommandingServiceBase::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != returnvalue::OK) {
    return result;
  }

  if (packetDestination == objects::NO_OBJECT) {
    packetDestination = defaultPacketDestination;
  }
  auto* packetForwarding = ObjectManager::instance()->get<AcceptsTelemetryIF>(packetDestination);

  if (packetSource == objects::NO_OBJECT) {
    packetSource = defaultPacketSource;
  }
  auto* distributor = ObjectManager::instance()->get<PusDistributorIF>(packetSource);

  if (packetForwarding == nullptr or distributor == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CommandingServiceBase::intialize: Packet source or "
                  "packet destination invalid!"
               << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  distributor->registerService(*this);
  requestQueue->setDefaultDestination(packetForwarding->getReportReceptionQueue());

  ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
  tcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);

  if (ipcStore == nullptr or tcStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CommandingServiceBase::intialize: IPC store or TC store "
                  "not initialized yet!"
               << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  if (tmStoreHelper.getTmStore() == nullptr) {
    auto* tmStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TM_STORE);
    if (tmStore == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
    tmStoreHelper.setTmStore(*tmStore);
  }
  // Generally, all TM packets will pass through a layer where the sequence count is set.
  // This avoids duplicate calculation of the CRC16
  tmStoreHelper.disableCrcCalculation();
  if (tmTimeStamper == nullptr) {
    tmTimeStamper = ObjectManager::instance()->get<TimeWriterIF>(objects::TIME_STAMPER);
    if (tmTimeStamper == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  }
  tmStoreHelper.setTimeStamper(*tmTimeStamper);

  if (errReporter == nullptr) {
    errReporter =
        ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER);
    if (errReporter != nullptr) {
      tmSendHelper.setInternalErrorReporter(*errReporter);
    }
  } else {
    tmSendHelper.setInternalErrorReporter(*errReporter);
  }
  tmSendHelper.setMsgQueue(*requestQueue);

  if (verificationReporter == nullptr) {
    verificationReporter =
        ObjectManager::instance()->get<VerificationReporterIF>(objects::VERIFICATION_REPORTER);
    if (verificationReporter == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  }
  return returnvalue::OK;
}

void CommandingServiceBase::handleCommandQueue() {
  CommandMessage reply;
  ReturnValue_t result;
  while (true) {
    result = commandQueue->receiveMessage(&reply);
    if (result == returnvalue::OK) {
      handleCommandMessage(&reply);
      continue;
    } else if (result == MessageQueueIF::EMPTY) {
      break;
    } else {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "CommandingServiceBase::handleCommandQueue: Receiving message failed"
                      "with code"
                   << result << std::endl;
#else
      sif::printWarning(
          "CommandingServiceBase::handleCommandQueue: Receiving message "
          "failed with code %d\n",
          result);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
      break;
    }
  }
}

void CommandingServiceBase::handleCommandMessage(CommandMessage* reply) {
  bool isStep = false;
  CommandMessage nextCommand;
  CommandMapIter iter = commandMap.find(reply->getSender());

  // handle unrequested reply first
  if (reply->getSender() == MessageQueueIF::NO_QUEUE or iter == commandMap.end()) {
    handleUnrequestedReply(reply);
    return;
  }
  nextCommand.setCommand(CommandMessage::CMD_NONE);

  // Implemented by child class, specifies what to do with reply.
  ReturnValue_t result = handleReply(reply, iter->second.command, &iter->second.state, &nextCommand,
                                     iter->second.objectId, &isStep);

  /* If the child implementation does not implement special handling for
   * rejected replies (returnvalue::FAILED or INVALID_REPLY is returned), a
   * failure verification will be generated with the reason as the
   * return code and the initial command as failure parameter 1 */
  if ((reply->getCommand() == CommandMessage::REPLY_REJECTED) and
      (result == returnvalue::FAILED or result == INVALID_REPLY)) {
    result = reply->getReplyRejectedReason();
    failureParameter1 = iter->second.command;
  }

  switch (result) {
    case EXECUTION_COMPLETE:
    case returnvalue::OK:
    case NO_STEP_MESSAGE:
      // handle result of reply handler implemented by developer.
      handleReplyHandlerResult(result, iter, &nextCommand, reply, isStep);
      break;
    case INVALID_REPLY:
      // might be just an unrequested reply at a bad moment
      handleUnrequestedReply(reply);
      break;
    default:
      if (isStep) {
        prepareVerificationFailureWithFullInfo(tcverif::PROGRESS_FAILURE, iter->second.tcInfo,
                                               result, true);
        failParams.step = ++iter->second.step;
      } else {
        prepareVerificationFailureWithFullInfo(tcverif::COMPLETION_FAILURE, iter->second.tcInfo,
                                               result, true);
      }
      verificationReporter->sendFailureReport(failParams);
      failureParameter1 = 0;
      failureParameter2 = 0;
      checkAndExecuteFifo(iter);
      break;
  }
}

void CommandingServiceBase::handleReplyHandlerResult(ReturnValue_t result, CommandMapIter iter,
                                                     CommandMessage* nextCommand,
                                                     CommandMessage* reply, bool& isStep) {
  iter->second.command = nextCommand->getCommand();

  // In case a new command is to be sent immediately, this is performed here.
  // If no new command is sent, only analyse reply result by initializing
  // sendResult as RETURN_OK
  ReturnValue_t sendResult = returnvalue::OK;
  if (nextCommand->getCommand() != CommandMessage::CMD_NONE) {
    sendResult = commandQueue->sendMessage(reply->getSender(), nextCommand);
  }

  if (sendResult == returnvalue::OK) {
    if (isStep and result != NO_STEP_MESSAGE) {
      prepareVerificationSuccessWithFullInfo(tcverif::PROGRESS_SUCCESS, iter->second.tcInfo);
      successParams.step = ++iter->second.step;
      verificationReporter->sendSuccessReport(successParams);
    } else {
      prepareVerificationSuccessWithFullInfo(tcverif::COMPLETION_SUCCESS, iter->second.tcInfo);
      verificationReporter->sendSuccessReport(successParams);
      checkAndExecuteFifo(iter);
    }
  } else {
    if (isStep) {
      prepareVerificationFailureWithFullInfo(tcverif::PROGRESS_FAILURE, iter->second.tcInfo, result,
                                             true);
      failParams.step = ++iter->second.step;
      nextCommand->clearCommandMessage();
      verificationReporter->sendFailureReport(failParams);
    } else {
      prepareVerificationFailureWithFullInfo(tcverif::COMPLETION_FAILURE, iter->second.tcInfo,
                                             result, true);
      nextCommand->clearCommandMessage();
      verificationReporter->sendFailureReport(failParams);
    }
    failureParameter1 = 0;
    failureParameter2 = 0;
    checkAndExecuteFifo(iter);
  }
}

void CommandingServiceBase::handleRequestQueue() {
  TmTcMessage message;
  ReturnValue_t result;
  store_address_t address;
  MessageQueueId_t queue;
  object_id_t objectId;
  for (result = requestQueue->receiveMessage(&message); result == returnvalue::OK;
       result = requestQueue->receiveMessage(&message)) {
    address = message.getStorageId();
    result = setUpTcReader(address);
    if (result != returnvalue::OK) {
      rejectPacketInvalidTc(result, address);
      continue;
    }
    if ((tcReader.getSubService() == 0) or
        (isValidSubservice(tcReader.getSubService()) != returnvalue::OK)) {
      rejectPacket(tcverif::START_FAILURE, address, INVALID_SUBSERVICE);
      continue;
    }

    result = getMessageQueueAndObject(tcReader.getSubService(), tcReader.getUserData(),
                                      tcReader.getUserDataLen(), &queue, &objectId);
    if (result != returnvalue::OK) {
      rejectPacket(tcverif::START_FAILURE, address, result);
      continue;
    }

    // Is a command already active for the target object?
    CommandMapIter iter;
    iter = commandMap.find(queue);

    if (iter != commandMap.end()) {
      result = iter->second.fifo.insert(address);
      if (result != returnvalue::OK) {
        rejectPacket(tcverif::START_FAILURE, address, OBJECT_BUSY);
      }
    } else {
      CommandInfo newInfo;  // Info will be set by startExecution if neccessary
      newInfo.objectId = objectId;
      result = commandMap.insert(queue, newInfo, &iter);
      if (result != returnvalue::OK) {
        rejectPacket(tcverif::START_FAILURE, address, BUSY);
      } else {
        startExecution(address, iter);
      }
    }
  }
}

ReturnValue_t CommandingServiceBase::sendTmPacket(uint8_t subservice, const uint8_t* sourceData,
                                                  size_t sourceDataLen) {
  ReturnValue_t result = tmHelper.prepareTmPacket(subservice, sourceData, sourceDataLen);
  if (result != returnvalue::OK) {
    return result;
  }
  return tmHelper.storeAndSendTmPacket();
}

ReturnValue_t CommandingServiceBase::sendTmPacket(uint8_t subservice, object_id_t objectId,
                                                  const uint8_t* data, size_t dataLen) {
  telemetry::DataWithObjectIdPrefix dataWithObjId(objectId, data, dataLen);
  ReturnValue_t result = tmHelper.prepareTmPacket(subservice, dataWithObjId);
  if (result != returnvalue::OK) {
    return result;
  }
  return tmHelper.storeAndSendTmPacket();
}

ReturnValue_t CommandingServiceBase::sendTmPacket(uint8_t subservice, SerializeIF& sourceData) {
  ReturnValue_t result = tmHelper.prepareTmPacket(subservice, sourceData);
  if (result != returnvalue::OK) {
    return result;
  }
  return tmHelper.storeAndSendTmPacket();
}

void CommandingServiceBase::startExecution(store_address_t storeId, CommandMapIter& iter) {
  CommandMessage command;
  iter->second.subservice = tcReader.getSubService();
  ReturnValue_t result =
      prepareCommand(&command, iter->second.subservice, tcReader.getUserData(),
                     tcReader.getUserDataLen(), &iter->second.state, iter->second.objectId);

  ReturnValue_t sendResult = returnvalue::OK;
  switch (result) {
    case returnvalue::OK:
      if (command.getCommand() != CommandMessage::CMD_NONE) {
        sendResult = commandQueue->sendMessage(iter.value->first, &command);
      }
      if (sendResult == returnvalue::OK) {
        Clock::getUptime(&iter->second.uptimeOfStart);
        iter->second.step = 0;
        iter->second.subservice = tcReader.getSubService();
        iter->second.command = command.getCommand();
        iter->second.tcInfo.ackFlags = tcReader.getAcknowledgeFlags();
        iter->second.tcInfo.tcPacketId = tcReader.getPacketIdRaw();
        iter->second.tcInfo.tcSequenceControl = tcReader.getPacketSeqCtrlRaw();
        acceptPacket(tcverif::START_SUCCESS, storeId);
      } else {
        command.clearCommandMessage();
        rejectPacket(tcverif::START_FAILURE, storeId, sendResult);
        checkAndExecuteFifo(iter);
      }
      break;
    case EXECUTION_COMPLETE:
      if (command.getCommand() != CommandMessage::CMD_NONE) {
        // Fire-and-forget command.
        sendResult = commandQueue->sendMessage(iter.value->first, &command);
      }
      if (sendResult == returnvalue::OK) {
        verificationReporter->sendSuccessReport(
            VerifSuccessParams(tcverif::START_SUCCESS, tcReader));
        acceptPacket(tcverif::COMPLETION_SUCCESS, storeId);
        checkAndExecuteFifo(iter);
      } else {
        command.clearCommandMessage();
        rejectPacket(tcverif::START_FAILURE, storeId, sendResult);
        checkAndExecuteFifo(iter);
      }
      break;
    default:
      rejectPacket(tcverif::START_FAILURE, storeId, result);
      checkAndExecuteFifo(iter);
      break;
  }
}

ReturnValue_t CommandingServiceBase::rejectPacketInvalidTc(ReturnValue_t errorCode,
                                                           store_address_t tcStoreId) {
  failureParameter1 = INVALID_TC;
  prepareVerificationFailureWithNoTcInfo(tcverif::START_FAILURE, errorCode, true);
  if (tcStoreId != store_address_t::invalid()) {
    tcStore->deleteData(tcStoreId);
  }
  return verificationReporter->sendFailureReport(failParams);
}

ReturnValue_t CommandingServiceBase::rejectPacket(uint8_t reportId, store_address_t tcStoreId,
                                                  ReturnValue_t errorCode) {
  ReturnValue_t result =
      verificationReporter->sendFailureReport(VerifFailureParams(reportId, tcReader, errorCode));
  tcStore->deleteData(tcStoreId);
  return result;
}

ReturnValue_t CommandingServiceBase::acceptPacket(uint8_t reportId, store_address_t tcStoreId) {
  ReturnValue_t result =
      verificationReporter->sendSuccessReport(VerifSuccessParams(reportId, tcReader));
  tcStore->deleteData(tcStoreId);
  return result;
}

void CommandingServiceBase::checkAndExecuteFifo(CommandMapIter& iter) {
  store_address_t address;
  if (iter->second.fifo.retrieve(&address) != returnvalue::OK) {
    commandMap.erase(&iter);
  } else {
    ReturnValue_t result = setUpTcReader(address);
    if (result == returnvalue::OK) {
      startExecution(address, iter);
    } else {
      // TODO: Warning?
      rejectPacket(tcverif::START_FAILURE, address, result);
    }
  }
}

void CommandingServiceBase::handleUnrequestedReply(CommandMessage* reply) {
  reply->clearCommandMessage();
}

inline void CommandingServiceBase::doPeriodicOperation() {}

MessageQueueId_t CommandingServiceBase::getCommandQueue() { return commandQueue->getId(); }

void CommandingServiceBase::checkTimeout() {
  uint32_t uptime;
  Clock::getUptime(&uptime);
  CommandMapIter iter;
  for (iter = commandMap.begin(); iter != commandMap.end(); ++iter) {
    if ((iter->second.uptimeOfStart + (timeoutSeconds * 1000)) < uptime) {
      prepareVerificationFailureWithFullInfo(tcverif::COMPLETION_FAILURE, iter->second.tcInfo,
                                             TIMEOUT, false);
      verificationReporter->sendFailureReport(failParams);
      checkAndExecuteFifo(iter);
    }
  }
}

void CommandingServiceBase::setTaskIF(PeriodicTaskIF* task_) { executingTask = task_; }

void CommandingServiceBase::setCustomTmStore(StorageManagerIF& store) {
  tmStoreHelper.setTmStore(store);
}

ReturnValue_t CommandingServiceBase::setUpTcReader(store_address_t storeId) {
  return tc::prepareTcReader(*tcStore, storeId, tcReader);
}

void CommandingServiceBase::prepareVerificationFailureWithNoTcInfo(uint8_t reportId,
                                                                   ReturnValue_t errorCode,
                                                                   bool setCachedFailParams) {
  failParams.resetTcFields();
  failParams.resetFailParams();
  failParams.reportId = reportId;
  failParams.errorCode = errorCode;
  if (setCachedFailParams) {
    failParams.errorParam1 = failureParameter1;
    failParams.errorParam2 = failureParameter2;
  }
}
void CommandingServiceBase::prepareVerificationFailureWithFullInfo(uint8_t reportId,
                                                                   CommandInfo::TcInfo& tcInfo,
                                                                   ReturnValue_t errorCode,
                                                                   bool setCachedFailParams) {
  failParams.reportId = reportId;
  failParams.tcPacketId = tcInfo.tcPacketId;
  failParams.tcPsc = tcInfo.tcSequenceControl;
  failParams.ackFlags = tcInfo.ackFlags;
  failParams.resetFailParams();
  failParams.errorCode = errorCode;
  if (setCachedFailParams) {
    failParams.errorParam1 = failureParameter1;
    failParams.errorParam2 = failureParameter2;
  }
}
void CommandingServiceBase::prepareVerificationSuccessWithFullInfo(
    uint8_t reportId, CommandingServiceBase::CommandInfo::TcInfo& tcInfo) {
  successParams.reportId = reportId;
  successParams.tcPacketId = tcInfo.tcPacketId;
  successParams.tcPsc = tcInfo.tcSequenceControl;
  successParams.ackFlags = tcInfo.ackFlags;
}

const char* CommandingServiceBase::getName() const { return name; }
