#include "fsfw/tmtcservices/CommandingServiceBase.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tcdistribution/PUSDistributorIF.h"
#include "fsfw/tmtcpacket/pus/tc.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"
#include "fsfw/tmtcservices/TmTcMessage.h"
#include "fsfw/tmtcservices/tcHelpers.h"
#include "fsfw/tmtcservices/tmHelpers.h"

object_id_t CommandingServiceBase::defaultPacketSource = objects::NO_OBJECT;
object_id_t CommandingServiceBase::defaultPacketDestination = objects::NO_OBJECT;

CommandingServiceBase::CommandingServiceBase(object_id_t setObjectId, uint16_t apid,
                                             uint8_t service, uint8_t numberOfParallelCommands,
                                             uint16_t commandTimeoutSeconds, size_t queueDepth)
    : SystemObject(setObjectId),
      apid(apid),
      service(service),
      timeoutSeconds(commandTimeoutSeconds),
      tmStoreHelper(apid),
      tmHelper(service, tmStoreHelper, tmSendHelper),
      commandMap(numberOfParallelCommands) {
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
  return RETURN_OK;
}

uint16_t CommandingServiceBase::getIdentifier() { return service; }

MessageQueueId_t CommandingServiceBase::getRequestQueue() { return requestQueue->getId(); }

ReturnValue_t CommandingServiceBase::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }

  if (packetDestination == objects::NO_OBJECT) {
    packetDestination = defaultPacketDestination;
  }
  auto* packetForwarding = ObjectManager::instance()->get<AcceptsTelemetryIF>(packetDestination);

  if (packetSource == objects::NO_OBJECT) {
    packetSource = defaultPacketSource;
  }
  auto* distributor = ObjectManager::instance()->get<PUSDistributorIF>(packetSource);

  if (packetForwarding == nullptr or distributor == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CommandingServiceBase::intialize: Packet source or "
                  "packet destination invalid!"
               << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  distributor->registerService(this);
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
  if (errReporter == nullptr) {
    errReporter =
        ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER);
    if (errReporter != nullptr) {
      tmSendHelper.setInternalErrorReporter(errReporter);
    }
  }
  return RETURN_OK;
}

void CommandingServiceBase::handleCommandQueue() {
  CommandMessage reply;
  ReturnValue_t result = RETURN_FAILED;
  while (true) {
    result = commandQueue->receiveMessage(&reply);
    if (result == HasReturnvaluesIF::RETURN_OK) {
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
   * rejected replies (RETURN_FAILED or INVALID_REPLY is returned), a
   * failure verification will be generated with the reason as the
   * return code and the initial command as failure parameter 1 */
  if ((reply->getCommand() == CommandMessage::REPLY_REJECTED) and
      (result == RETURN_FAILED or result == INVALID_REPLY)) {
    result = reply->getReplyRejectedReason();
    failureParameter1 = iter->second.command;
  }

  switch (result) {
    case EXECUTION_COMPLETE:
    case RETURN_OK:
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
        verificationReporter.sendFailureReport(
            tcverif::PROGRESS_FAILURE, iter->second.tcInfo.ackFlags, iter->second.tcInfo.tcPacketId,
            iter->second.tcInfo.tcSequenceControl, result, ++iter->second.step, failureParameter1,
            failureParameter2);
      } else {
        verificationReporter.sendFailureReport(
            tcverif::COMPLETION_FAILURE, iter->second.tcInfo.ackFlags,
            iter->second.tcInfo.tcPacketId, iter->second.tcInfo.tcSequenceControl, result, 0,
            failureParameter1, failureParameter2);
      }
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
  ReturnValue_t sendResult = RETURN_OK;
  if (nextCommand->getCommand() != CommandMessage::CMD_NONE) {
    sendResult = commandQueue->sendMessage(reply->getSender(), nextCommand);
  }

  if (sendResult == RETURN_OK) {
    if (isStep and result != NO_STEP_MESSAGE) {
      verificationReporter.sendSuccessReport(
          tcverif::PROGRESS_SUCCESS, iter->second.tcInfo.ackFlags, iter->second.tcInfo.tcPacketId,
          iter->second.tcInfo.tcSequenceControl, ++iter->second.step);
    } else {
      verificationReporter.sendSuccessReport(
          tcverif::COMPLETION_SUCCESS, iter->second.tcInfo.ackFlags, iter->second.tcInfo.tcPacketId,
          iter->second.tcInfo.tcSequenceControl, 0);
      checkAndExecuteFifo(iter);
    }
  } else {
    if (isStep) {
      nextCommand->clearCommandMessage();
      verificationReporter.sendFailureReport(
          tcverif::PROGRESS_FAILURE, iter->second.tcInfo.ackFlags, iter->second.tcInfo.tcPacketId,
          iter->second.tcInfo.tcSequenceControl, sendResult, ++iter->second.step, failureParameter1,
          failureParameter2);
    } else {
      nextCommand->clearCommandMessage();
      verificationReporter.sendFailureReport(
          tcverif::COMPLETION_FAILURE, iter->second.tcInfo.ackFlags, iter->second.tcInfo.tcPacketId,
          iter->second.tcInfo.tcSequenceControl, sendResult, 0, failureParameter1,
          failureParameter2);
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
  for (result = requestQueue->receiveMessage(&message); result == RETURN_OK;
       result = requestQueue->receiveMessage(&message)) {
    result = setUpTcReader(message.getStorageId());
    if (result != HasReturnvaluesIF::RETURN_OK) {
      // TODO: Warning?
      rejectPacket(tcverif::START_FAILURE, address, &tcReader, result);
      continue;
    }
    if ((tcReader.getSubService() == 0) or
        (isValidSubservice(tcReader.getSubService()) != RETURN_OK)) {
      rejectPacket(tcverif::START_FAILURE, address, &tcReader, INVALID_SUBSERVICE);
      continue;
    }

    result = getMessageQueueAndObject(tcReader.getSubService(), tcReader.getUserData(),
                                      tcReader.getUserDataLen(), &queue, &objectId);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      rejectPacket(tcverif::START_FAILURE, address, &tcReader, result);
      continue;
    }

    // Is a command already active for the target object?
    CommandMapIter iter;
    iter = commandMap.find(queue);

    if (iter != commandMap.end()) {
      result = iter->second.fifo.insert(address);
      if (result != RETURN_OK) {
        rejectPacket(tcverif::START_FAILURE, address, &tcReader, OBJECT_BUSY);
      }
    } else {
      CommandInfo newInfo;  // Info will be set by startExecution if neccessary
      newInfo.objectId = objectId;
      result = commandMap.insert(queue, newInfo, &iter);
      if (result != RETURN_OK) {
        rejectPacket(tcverif::START_FAILURE, address, &tcReader, BUSY);
      } else {
        startExecution(address, iter);
      }
    }
  }
}

ReturnValue_t CommandingServiceBase::sendTmPacket(uint8_t subservice, const uint8_t* sourceData,
                                                  size_t sourceDataLen) {
  return tmHelper.sendTmPacket(subservice, sourceData, sourceDataLen);
}

ReturnValue_t CommandingServiceBase::sendTmPacket(uint8_t subservice, object_id_t objectId,
                                                  const uint8_t* data, size_t dataLen) {
  return tmHelper.sendTmPacket(subservice, objectId, data, dataLen);
}

ReturnValue_t CommandingServiceBase::sendTmPacket(uint8_t subservice, SerializeIF& sourceData) {
  return tmHelper.sendTmPacket(subservice, sourceData);
}

void CommandingServiceBase::startExecution(store_address_t storeId, CommandMapIter iter) {
  ReturnValue_t result = RETURN_OK;
  CommandMessage command;
  iter->second.subservice = tcReader.getSubService();
  result = prepareCommand(&command, iter->second.subservice, tcReader.getUserData(),
                          tcReader.getUserDataLen(), &iter->second.state, iter->second.objectId);

  ReturnValue_t sendResult = RETURN_OK;
  switch (result) {
    case RETURN_OK:
      if (command.getCommand() != CommandMessage::CMD_NONE) {
        sendResult = commandQueue->sendMessage(iter.value->first, &command);
      }
      if (sendResult == RETURN_OK) {
        Clock::getUptime(&iter->second.uptimeOfStart);
        iter->second.step = 0;
        iter->second.subservice = tcReader.getSubService();
        iter->second.command = command.getCommand();
        iter->second.tcInfo.ackFlags = tcReader.getAcknowledgeFlags();
        iter->second.tcInfo.tcPacketId = tcReader.getPacketIdRaw();
        iter->second.tcInfo.tcSequenceControl = tcReader.getPacketSeqCtrlRaw();
        acceptPacket(tcverif::START_SUCCESS, storeId, &tcReader);
      } else {
        command.clearCommandMessage();
        rejectPacket(tcverif::START_FAILURE, storeId, &tcReader, sendResult);
        checkAndExecuteFifo(iter);
      }
      break;
    case EXECUTION_COMPLETE:
      if (command.getCommand() != CommandMessage::CMD_NONE) {
        // Fire-and-forget command.
        sendResult = commandQueue->sendMessage(iter.value->first, &command);
      }
      if (sendResult == RETURN_OK) {
        verificationReporter.sendSuccessReport(tcverif::START_SUCCESS, &tcReader);
        acceptPacket(tcverif::COMPLETION_SUCCESS, storeId, &tcReader);
        checkAndExecuteFifo(iter);
      } else {
        command.clearCommandMessage();
        rejectPacket(tcverif::START_FAILURE, storeId, &tcReader, sendResult);
        checkAndExecuteFifo(iter);
      }
      break;
    default:
      rejectPacket(tcverif::START_FAILURE, storeId, &tcReader, result);
      checkAndExecuteFifo(iter);
      break;
  }
}

void CommandingServiceBase::rejectPacket(uint8_t reportId, store_address_t tcStoreId,
                                         PusTcReader* correspondingTc, ReturnValue_t errorCode) {
  verificationReporter.sendFailureReport(reportId, correspondingTc, errorCode);
  tcStore->deleteData(tcStoreId);
}

void CommandingServiceBase::acceptPacket(uint8_t reportId, store_address_t tcStoreId,
                                         PusTcReader* packet) {
  verificationReporter.sendSuccessReport(reportId, packet);
  tcStore->deleteData(tcStoreId);
}

void CommandingServiceBase::checkAndExecuteFifo(CommandMapIter& iter) {
  store_address_t address;
  if (iter->second.fifo.retrieve(&address) != RETURN_OK) {
    commandMap.erase(&iter);
  } else {
    ReturnValue_t result = setUpTcReader(address);
    if (result == HasReturnvaluesIF::RETURN_OK) {
      startExecution(address, iter);
    } else {
      // TODO: Warning?
      rejectPacket(tcverif::START_FAILURE, address, &tcReader, result);
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
      verificationReporter.sendFailureReport(
          tcverif::COMPLETION_FAILURE, iter->second.tcInfo.ackFlags, iter->second.tcInfo.tcPacketId,
          iter->second.tcInfo.tcSequenceControl, TIMEOUT);
      checkAndExecuteFifo(iter);
    }
  }
}

void CommandingServiceBase::setTaskIF(PeriodicTaskIF* task_) { executingTask = task_; }

void CommandingServiceBase::setCustomTmStore(StorageManagerIF& store) {
  tmStoreHelper.setTmStore(store);
}
ReturnValue_t CommandingServiceBase::setUpTcReader(store_address_t storeId) {
  return tc::prepareTcReader(tcStore, storeId, tcReader);
}
