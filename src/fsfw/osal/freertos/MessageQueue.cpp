#include "fsfw/osal/freertos/MessageQueue.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/osal/freertos/QueueMapManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

MessageQueue::MessageQueue(size_t messageDepth, size_t maxMessageSize, MqArgs* args)
    : MessageQueueBase(MessageQueueIF::NO_QUEUE, MessageQueueIF::NO_QUEUE, args),
      maxMessageSize(maxMessageSize) {
  handle = xQueueCreate(messageDepth, maxMessageSize);
  if (handle == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "MessageQueue::MessageQueue: Creation failed" << std::endl;
    sif::error << "Specified Message Depth: " << messageDepth << std::endl;
    sif::error << "Specified Maximum Message Size: " << maxMessageSize << std::endl;
#else
    sif::printError("MessageQueue::MessageQueue: Creation failed\n");
    sif::printError("Specified Message Depth: %d\n", messageDepth);
    sif::printError("Specified Maximum Message Size: %d\n", maxMessageSize);
#endif
  }
  QueueMapManager::instance()->addMessageQueue(handle, &id);
}

MessageQueue::~MessageQueue() {
  if (handle != nullptr) {
    vQueueDelete(handle);
  }
}

void MessageQueue::switchSystemContext(CallContext callContext) { this->callContext = callContext; }

ReturnValue_t MessageQueue::sendMessageFrom(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                            MessageQueueId_t sentFrom, bool ignoreFault) {
  return sendMessageFromMessageQueue(sendTo, message, sentFrom, ignoreFault, callContext);
}

QueueHandle_t MessageQueue::getNativeQueueHandle() { return handle; }

ReturnValue_t MessageQueue::handleSendResult(BaseType_t result, bool ignoreFault) {
  if (result != pdPASS) {
    if (not ignoreFault) {
      InternalErrorReporterIF* internalErrorReporter =
          ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER);
      if (internalErrorReporter != nullptr) {
        internalErrorReporter->queueMessageNotSent();
      }
    }
    return MessageQueueIF::FULL;
  }
  return returnvalue::OK;
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessageIF* message) {
  BaseType_t result = xQueueReceive(handle, reinterpret_cast<void*>(message->getBuffer()), 0);
  if (result == pdPASS) {
    this->last = message->getSender();
    return returnvalue::OK;
  } else {
    return MessageQueueIF::EMPTY;
  }
}

ReturnValue_t MessageQueue::flush(uint32_t* count) {
  // TODO FreeRTOS does not support flushing partially
  // Is always successful
  xQueueReset(handle);
  return returnvalue::OK;
}

// static core function to send messages.
ReturnValue_t MessageQueue::sendMessageFromMessageQueue(MessageQueueId_t sendTo,
                                                        MessageQueueMessageIF* message,
                                                        MessageQueueId_t sentFrom, bool ignoreFault,
                                                        CallContext callContext) {
  BaseType_t result = pdFALSE;
  if (sendTo == MessageQueueIF::NO_QUEUE) {
    return MessageQueueIF::DESTINATION_INVALID;
  }

  QueueHandle_t destination = QueueMapManager::instance()->getMessageQueue(sendTo);
  if (destination == nullptr) {
    return MessageQueueIF::DESTINATION_INVALID;
  }

  message->setSender(sentFrom);
  if (callContext == CallContext::TASK) {
    result = xQueueSendToBack(destination, static_cast<const void*>(message->getBuffer()), 0);
  } else {
    /* If the call context is from an interrupt, request a context switch if a higher priority
    task was blocked by the interrupt. */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    result = xQueueSendFromISR(destination, static_cast<const void*>(message->getBuffer()),
                               &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
      TaskManagement::requestContextSwitch(callContext);
    }
  }
  return handleSendResult(result, ignoreFault);
}
