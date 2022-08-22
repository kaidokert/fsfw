#include "fsfw/osal/rtems/MessageQueue.h"

#include <cstring>

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/osal/rtems/RtemsBasic.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

MessageQueue::MessageQueue(size_t message_depth, size_t max_message_size, MqArgs* args)
    : MessageQueueBase(MessageQueueIF::NO_QUEUE, MessageQueueIF::NO_QUEUE, args),
      internalErrorReporter(nullptr) {
  rtems_name name = ('Q' << 24) + (queueCounter++ << 8);
  rtems_status_code status =
      rtems_message_queue_create(name, message_depth, max_message_size, 0, &(this->id));
  if (status != RTEMS_SUCCESSFUL) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "MessageQueue::MessageQueue: Creating Queue " << std::hex << name << std::dec
               << " failed with status:" << (uint32_t)status << std::endl;
#endif
    this->id = MessageQueueIF::NO_QUEUE;
  }
}

MessageQueue::~MessageQueue() { rtems_message_queue_delete(id); }

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessageIF* message) {
  size_t size = 0;
  rtems_status_code status =
      rtems_message_queue_receive(id, message->getBuffer(), &size, RTEMS_NO_WAIT, 1);
  if (status == RTEMS_SUCCESSFUL) {
    message->setMessageSize(size);
    this->last = message->getSender();
    // Check size of incoming message.
    if (message->getMessageSize() < message->getMinimumMessageSize()) {
      return returnvalue::FAILED;
    }
  } else {
    // No message was received. Keep lastPartner anyway, I might send something later.
    // But still, delete packet content.
    memset(message->getData(), 0, message->getMaximumDataSize());
  }
  return convertReturnCode(status);
}

ReturnValue_t MessageQueue::flush(uint32_t* count) {
  rtems_status_code status = rtems_message_queue_flush(id, count);
  return convertReturnCode(status);
}

ReturnValue_t MessageQueue::sendMessageFrom(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                            MessageQueueId_t sentFrom, bool ignoreFault) {
  message->setSender(sentFrom);
  rtems_status_code result =
      rtems_message_queue_send(sendTo, message->getBuffer(), message->getMessageSize());

  // TODO: Check if we're in ISR.
  if (result != RTEMS_SUCCESSFUL && !ignoreFault) {
    if (internalErrorReporter == nullptr) {
      internalErrorReporter =
          ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER);
    }
    if (internalErrorReporter != nullptr) {
      internalErrorReporter->queueMessageNotSent();
    }
  }

  ReturnValue_t returnCode = convertReturnCode(result);
  if (returnCode == MessageQueueIF::EMPTY) {
    return returnvalue::FAILED;
  }

  return returnCode;
}

ReturnValue_t MessageQueue::convertReturnCode(rtems_status_code inValue) {
  switch (inValue) {
    case RTEMS_SUCCESSFUL:
      return returnvalue::OK;
    case RTEMS_INVALID_ID:
      return returnvalue::FAILED;
    case RTEMS_TIMEOUT:
      return returnvalue::FAILED;
    case RTEMS_OBJECT_WAS_DELETED:
      return returnvalue::FAILED;
    case RTEMS_INVALID_ADDRESS:
      return returnvalue::FAILED;
    case RTEMS_INVALID_SIZE:
      return returnvalue::FAILED;
    case RTEMS_TOO_MANY:
      return MessageQueueIF::FULL;
    case RTEMS_UNSATISFIED:
      return MessageQueueIF::EMPTY;
    default:
      return returnvalue::FAILED;
  }
}

uint16_t MessageQueue::queueCounter = 0;
