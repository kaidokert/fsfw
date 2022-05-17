#ifndef FSFW_IPC_QUEUEFACTORY_H_
#define FSFW_IPC_QUEUEFACTORY_H_

#include <cstdint>

#include "MessageQueueIF.h"
#include "MessageQueueMessage.h"
#include "definitions.h"

/**
 * Creates message queues.
 * This class is a "singleton" interface, i.e. it provides an
 * interface, but also is the base class for a singleton.
 */
class QueueFactory {
 public:
  virtual ~QueueFactory();
  /**
   * Returns the single instance of QueueFactory.
   * The implementation of #instance is found in its subclasses.
   * Thus, we choose link-time variability of the  instance.
   */
  static QueueFactory* instance();

  MessageQueueIF* createMessageQueue(uint32_t messageDepth = 3,
                                     size_t maxMessageSize = MessageQueueMessage::MAX_MESSAGE_SIZE,
                                     MqArgs* args = nullptr);

  void deleteMessageQueue(MessageQueueIF* queue);

 private:
  /**
   * External instantiation is not allowed.
   */
  QueueFactory();
  static QueueFactory* factoryInstance;
};

#endif /* FSFW_IPC_QUEUEFACTORY_H_ */
