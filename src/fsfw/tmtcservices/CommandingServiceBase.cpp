#include "fsfw/tmtcservices/CommandingServiceBase.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tcdistribution/PUSDistributorIF.h"
#include "fsfw/tmtcpacket/pus/tc.h"
#include "fsfw/tmtcpacket/pus/tm.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

object_id_t CommandingServiceBase::defaultPacketSource = objects::NO_OBJECT;
object_id_t CommandingServiceBase::defaultPacketDestination = objects::NO_OBJECT;

CommandingServiceBase::CommandingServiceBase(object_id_t setObjectId, uint16_t apid,
                                             uint8_t service, uint8_t numberOfParallelCommands,
                                             uint16_t commandTimeoutSeconds, size_t queueDepth)
    : SystemObject(setObjectId),
      apid(apid),
      service(service),
      timeoutSeconds(commandTimeoutSeconds),
      commandMap(numberOfParallelCommands) {
  commandQueue = QueueFactory::instance()->createMessageQueue(queueDepth);
  requestQueue = QueueFactory::instance()->createMessageQueue(queueDepth);
}

void CommandingServiceBase::setPacketSource(object_id_t packetSource) {
  this->packetSource = packetSource;
}

void CommandingServiceBase::setPacketDestination(object_id_t packetDestination) {
  this->packetDestination = packetDestination;
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
  AcceptsTelemetryIF* packetForwarding =
      ObjectManager::instance()->get<AcceptsTelemetryIF>(packetDestination);

  if (packetSource == objects::NO_OBJECT) {
    packetSource = defaultPacketSource;
  }
  PUSDistributorIF* distributor = ObjectManager::instance()->get<PUSDistributorIF>(packetSource);

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

  IPCStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
  TCStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);

  if (IPCStore == nullptr or TCStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CommandingServiceBase::intialize: IPC store or TC store "
                  "not initialized yet!"
               << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
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
            tc_verification::PROGRESS_FAILURE, iter->second.tcInfo.ackFlags,
            iter->second.tcInfo.tcPacketId, iter->second.tcInfo.tcSequenceControl, result,
            ++iter->second.step, failureParameter1, failureParameter2);
      } else {
        verificationReporter.sendFailureReport(
            tc_verification::COMPLETION_FAILURE, iter->second.tcInfo.ackFlags,
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
          tc_verification::PROGRESS_SUCCESS, iter->second.tcInfo.ackFlags,
          iter->second.tcInfo.tcPacketId, iter->second.tcInfo.tcSequenceControl,
          ++iter->second.step);
    } else {
      verificationReporter.sendSuccessReport(
          tc_verification::COMPLETION_SUCCESS, iter->second.tcInfo.ackFlags,
          iter->second.tcInfo.tcPacketId, iter->second.tcInfo.tcSequenceControl, 0);
      checkAndExecuteFifo(iter);
    }
  } else {
    if (isStep) {
      nextCommand->clearCommandMessage();
      verificationReporter.sendFailureReport(
          tc_verification::PROGRESS_FAILURE, iter->second.tcInfo.ackFlags,
          iter->second.tcInfo.tcPacketId, iter->second.tcInfo.tcSequenceControl, sendResult,
          ++iter->second.step, failureParameter1, failureParameter2);
    } else {
      nextCommand->clearCommandMessage();
      verificationReporter.sendFailureReport(
          tc_verification::COMPLETION_FAILURE, iter->second.tcInfo.ackFlags,
          iter->second.tcInfo.tcPacketId, iter->second.tcInfo.tcSequenceControl, sendResult, 0,
          failureParameter1, failureParameter2);
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
  TcPacketStoredPus packet;
  MessageQueueId_t queue;
  object_id_t objectId;
  for (result = requestQueue->receiveMessage(&message); result == RETURN_OK;
       result = requestQueue->receiveMessage(&message)) {
    address = message.getStorageId();
    packet.setStoreAddress(address, &packet);

    if ((packet.getSubService() == 0) or (isValidSubservice(packet.getSubService()) != RETURN_OK)) {
      rejectPacket(tc_verification::START_FAILURE, &packet, INVALID_SUBSERVICE);
      continue;
    }

    result = getMessageQueueAndObject(packet.getSubService(), packet.getApplicationData(),
                                      packet.getApplicationDataSize(), &queue, &objectId);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      rejectPacket(tc_verification::START_FAILURE, &packet, result);
      continue;
    }

    // Is a command already active for the target object?
    CommandMapIter iter;
    iter = commandMap.find(queue);

    if (iter != commandMap.end()) {
      result = iter->second.fifo.insert(address);
      if (result != RETURN_OK) {
        rejectPacket(tc_verification::START_FAILURE, &packet, OBJECT_BUSY);
      }
    } else {
      CommandInfo newInfo;  // Info will be set by startExecution if neccessary
      newInfo.objectId = objectId;
      result = commandMap.insert(queue, newInfo, &iter);
      if (result != RETURN_OK) {
        rejectPacket(tc_verification::START_FAILURE, &packet, BUSY);
      } else {
        startExecution(&packet, iter);
      }
    }
  }
}

ReturnValue_t CommandingServiceBase::sendTmPacket(uint8_t subservice, const uint8_t* data,
                                                  size_t dataLen, const uint8_t* headerData,
                                                  size_t headerSize) {
#if FSFW_USE_PUS_C_TELEMETRY == 0
  TmPacketStoredPusA tmPacketStored(this->apid, this->service, subservice, this->tmPacketCounter,
                                    data, dataLen, headerData, headerSize);
#else
  TmPacketStoredPusC tmPacketStored(this->apid, this->service, subservice, this->tmPacketCounter,
                                    data, dataLen, headerData, headerSize);
#endif
  ReturnValue_t result =
      tmPacketStored.sendPacket(requestQueue->getDefaultDestination(), requestQueue->getId());
  if (result == HasReturnvaluesIF::RETURN_OK) {
    this->tmPacketCounter++;
  }
  return result;
}

ReturnValue_t CommandingServiceBase::sendTmPacket(uint8_t subservice, object_id_t objectId,
                                                  const uint8_t* data, size_t dataLen) {
  uint8_t buffer[sizeof(object_id_t)];
  uint8_t* pBuffer = buffer;
  size_t size = 0;
  SerializeAdapter::serialize(&objectId, &pBuffer, &size, sizeof(object_id_t),
                              SerializeIF::Endianness::BIG);
#if FSFW_USE_PUS_C_TELEMETRY == 0
  TmPacketStoredPusA tmPacketStored(this->apid, this->service, subservice, this->tmPacketCounter,
                                    data, dataLen, buffer, size);
#else
  TmPacketStoredPusC tmPacketStored(this->apid, this->service, subservice, this->tmPacketCounter,
                                    data, dataLen, buffer, size);
#endif
  ReturnValue_t result =
      tmPacketStored.sendPacket(requestQueue->getDefaultDestination(), requestQueue->getId());
  if (result == HasReturnvaluesIF::RETURN_OK) {
    this->tmPacketCounter++;
  }
  return result;
}

ReturnValue_t CommandingServiceBase::sendTmPacket(uint8_t subservice, SerializeIF* content,
                                                  SerializeIF* header) {
#if FSFW_USE_PUS_C_TELEMETRY == 0
  TmPacketStoredPusA tmPacketStored(this->apid, this->service, subservice, this->tmPacketCounter,
                                    content, header);
#else
  TmPacketStoredPusC tmPacketStored(this->apid, this->service, subservice, this->tmPacketCounter,
                                    content, header);
#endif
  ReturnValue_t result =
      tmPacketStored.sendPacket(requestQueue->getDefaultDestination(), requestQueue->getId());
  if (result == HasReturnvaluesIF::RETURN_OK) {
    this->tmPacketCounter++;
  }
  return result;
}

void CommandingServiceBase::startExecution(TcPacketStoredPus* storedPacket, CommandMapIter iter) {
  ReturnValue_t result = RETURN_OK;
  CommandMessage command;
  // TcPacketPusBase* tcPacketBase = storedPacket->getPacketBase();
  if (storedPacket == nullptr) {
    return;
  }
  iter->second.subservice = storedPacket->getSubService();
  result = prepareCommand(&command, iter->second.subservice, storedPacket->getApplicationData(),
                          storedPacket->getApplicationDataSize(), &iter->second.state,
                          iter->second.objectId);

  ReturnValue_t sendResult = RETURN_OK;
  switch (result) {
    case RETURN_OK:
      if (command.getCommand() != CommandMessage::CMD_NONE) {
        sendResult = commandQueue->sendMessage(iter.value->first, &command);
      }
      if (sendResult == RETURN_OK) {
        Clock::getUptime(&iter->second.uptimeOfStart);
        iter->second.step = 0;
        iter->second.subservice = storedPacket->getSubService();
        iter->second.command = command.getCommand();
        iter->second.tcInfo.ackFlags = storedPacket->getAcknowledgeFlags();
        iter->second.tcInfo.tcPacketId = storedPacket->getPacketId();
        iter->second.tcInfo.tcSequenceControl = storedPacket->getPacketSequenceControl();
        acceptPacket(tc_verification::START_SUCCESS, storedPacket);
      } else {
        command.clearCommandMessage();
        rejectPacket(tc_verification::START_FAILURE, storedPacket, sendResult);
        checkAndExecuteFifo(iter);
      }
      break;
    case EXECUTION_COMPLETE:
      if (command.getCommand() != CommandMessage::CMD_NONE) {
        // Fire-and-forget command.
        sendResult = commandQueue->sendMessage(iter.value->first, &command);
      }
      if (sendResult == RETURN_OK) {
        verificationReporter.sendSuccessReport(tc_verification::START_SUCCESS,
                                               storedPacket->getPacketBase());
        acceptPacket(tc_verification::COMPLETION_SUCCESS, storedPacket);
        checkAndExecuteFifo(iter);
      } else {
        command.clearCommandMessage();
        rejectPacket(tc_verification::START_FAILURE, storedPacket, sendResult);
        checkAndExecuteFifo(iter);
      }
      break;
    default:
      rejectPacket(tc_verification::START_FAILURE, storedPacket, result);
      checkAndExecuteFifo(iter);
      break;
  }
}

void CommandingServiceBase::rejectPacket(uint8_t reportId, TcPacketStoredPus* packet,
                                         ReturnValue_t errorCode) {
  verificationReporter.sendFailureReport(reportId, dynamic_cast<TcPacketPusBase*>(packet),
                                         errorCode);
  packet->deletePacket();
}

void CommandingServiceBase::acceptPacket(uint8_t reportId, TcPacketStoredPus* packet) {
  verificationReporter.sendSuccessReport(reportId, dynamic_cast<TcPacketPusBase*>(packet));
  packet->deletePacket();
}

void CommandingServiceBase::checkAndExecuteFifo(CommandMapIter& iter) {
  store_address_t address;
  if (iter->second.fifo.retrieve(&address) != RETURN_OK) {
    commandMap.erase(&iter);
  } else {
    TcPacketStoredPus newPacket(address);
    startExecution(&newPacket, iter);
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
          tc_verification::COMPLETION_FAILURE, iter->second.tcInfo.ackFlags,
          iter->second.tcInfo.tcPacketId, iter->second.tcInfo.tcSequenceControl, TIMEOUT);
      checkAndExecuteFifo(iter);
    }
  }
}

void CommandingServiceBase::setTaskIF(PeriodicTaskIF* task_) { executingTask = task_; }
