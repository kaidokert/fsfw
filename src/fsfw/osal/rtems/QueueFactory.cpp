#include "fsfw/ipc/QueueFactory.h"

#include "fsfw/ipc/MessageQueueSenderIF.h"
#include "fsfw/osal/rtems/MessageQueue.h"
#include "fsfw/osal/rtems/RtemsBasic.h"

QueueFactory* QueueFactory::factoryInstance = nullptr;

ReturnValue_t MessageQueueSenderIF::sendMessage(MessageQueueId_t sendTo,
                                                MessageQueueMessageIF* message,
                                                MessageQueueId_t sentFrom, bool ignoreFault) {
  // TODO add ignoreFault functionality
  message->setSender(sentFrom);
  rtems_status_code result =
      rtems_message_queue_send(sendTo, message->getBuffer(), message->getMessageSize());
  switch (result) {
    case RTEMS_SUCCESSFUL:
      // message sent successfully
      return returnvalue::OK;
    case RTEMS_INVALID_ID:
      // invalid queue id
      return returnvalue::FAILED;
    case RTEMS_INVALID_SIZE:
      // invalid message size
      return returnvalue::FAILED;
    case RTEMS_INVALID_ADDRESS:
      // buffer is NULL
      return returnvalue::FAILED;
    case RTEMS_UNSATISFIED:
      // out of message buffers
      return returnvalue::FAILED;
    case RTEMS_TOO_MANY:
      // queue's limit has been reached
      return MessageQueueIF::FULL;

    default:
      return returnvalue::FAILED;
  }
}

QueueFactory* QueueFactory::instance() {
  if (factoryInstance == nullptr) {
    factoryInstance = new QueueFactory;
  }
  return factoryInstance;
}

QueueFactory::QueueFactory() = default;

QueueFactory::~QueueFactory() = default;

MessageQueueIF* QueueFactory::createMessageQueue(uint32_t messageDepth, size_t maxMessageSize,
                                                 MqArgs* args) {
  return new MessageQueue(messageDepth, maxMessageSize, args);
}

void QueueFactory::deleteMessageQueue(MessageQueueIF* queue) { delete queue; }
