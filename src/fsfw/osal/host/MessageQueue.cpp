#include "fsfw/osal/host/MessageQueue.h"

#include <cstring>

#include "fsfw/ipc/MutexFactory.h"
#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/osal/host/QueueMapManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

MessageQueue::MessageQueue(size_t messageDepth, size_t maxMessageSize, MqArgs* args)
    : MessageQueueBase(MessageQueueIF::NO_QUEUE, MessageQueueIF::NO_QUEUE, args),
      messageSize(maxMessageSize),
      messageDepth(messageDepth) {
  queueLock = MutexFactory::instance()->createMutex();
  auto result = QueueMapManager::instance()->addMessageQueue(this, &id);
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "MessageQueue::MessageQueue: Could not be created" << std::endl;
#else
    sif::printError("MessageQueue::MessageQueue: Could not be created\n");
#endif
  }
}

MessageQueue::~MessageQueue() { MutexFactory::instance()->deleteMutex(queueLock); }

ReturnValue_t MessageQueue::sendMessageFrom(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                            MessageQueueId_t sentFrom, bool ignoreFault) {
  return sendMessageFromMessageQueue(sendTo, message, sentFrom, ignoreFault);
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessageIF* message) {
  if (messageQueue.empty()) {
    return MessageQueueIF::EMPTY;
  }
  MutexGuard mutexLock(queueLock, MutexIF::TimeoutType::WAITING, 20);
  std::copy(messageQueue.front().data(), messageQueue.front().data() + messageSize,
            message->getBuffer());
  messageQueue.pop();
  // The last partner is the first uint32_t field in the message
  this->last = message->getSender();
  return returnvalue::OK;
}

ReturnValue_t MessageQueue::flush(uint32_t* count) {
  *count = messageQueue.size();
  // Clears the queue.
  messageQueue = std::queue<std::vector<uint8_t>>();
  return returnvalue::OK;
}

// static core function to send messages.
ReturnValue_t MessageQueue::sendMessageFromMessageQueue(MessageQueueId_t sendTo,
                                                        MessageQueueMessageIF* message,
                                                        MessageQueueId_t sentFrom,
                                                        bool ignoreFault) {
  if (message == nullptr) {
    return returnvalue::FAILED;
  }
  message->setSender(sentFrom);
  if (message->getMessageSize() > message->getMaximumMessageSize()) {
    // Actually, this should never happen or an error will be emitted
    // in MessageQueueMessage.
    // But I will still return a failure here.
    return returnvalue::FAILED;
  }
  auto* targetQueue =
      dynamic_cast<MessageQueue*>(QueueMapManager::instance()->getMessageQueue(sendTo));
  if (targetQueue == nullptr) {
    if (not ignoreFault) {
      auto* internalErrorReporter =
          ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER);
      if (internalErrorReporter != nullptr) {
        internalErrorReporter->queueMessageNotSent();
      }
    }
    return MessageQueueIF::DESTINATION_INVALID;
  }
  if (targetQueue->messageQueue.size() < targetQueue->messageDepth) {
    MutexGuard mutexLock(targetQueue->queueLock, MutexIF::TimeoutType::WAITING, 20);
    targetQueue->messageQueue.push(std::vector<uint8_t>(message->getMaximumMessageSize()));
    memcpy(targetQueue->messageQueue.back().data(), message->getBuffer(),
           message->getMaximumMessageSize());
  } else {
    if (not ignoreFault) {
      auto* internalErrorReporter =
          ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER);
      if (internalErrorReporter != nullptr) {
        internalErrorReporter->queueMessageNotSent();
      }
    }
    return MessageQueueIF::FULL;
  }
  return returnvalue::OK;
}

ReturnValue_t MessageQueue::lockQueue(MutexIF::TimeoutType timeoutType, dur_millis_t lockTimeout) {
  return queueLock->lockMutex(timeoutType, lockTimeout);
}

ReturnValue_t MessageQueue::unlockQueue() { return queueLock->unlockMutex(); }
