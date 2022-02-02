#ifndef FSFW_OSAL_HOST_QUEUEMAPMANAGER_H_
#define FSFW_OSAL_HOST_QUEUEMAPMANAGER_H_

#include <atomic>
#include <unordered_map>

#include "../../ipc/MessageQueueSenderIF.h"
#include "../../osal/host/MessageQueue.h"

using QueueMap = std::unordered_map<MessageQueueId_t, MessageQueueIF*>;

/**
 * An internal map to map message queue IDs to message queues.
 * This propably should be a singleton..
 */
class QueueMapManager {
 public:
  //! Returns the single instance of QueueMapManager.
  static QueueMapManager* instance();

  /**
   * Insert a message queue into the map and returns a message queue ID
   * @param queue The message queue to insert.
   * @param id The passed value will be set unless a nullptr is passed
   * @return
   */
  ReturnValue_t addMessageQueue(MessageQueueIF* queue, MessageQueueId_t* id = nullptr);
  /**
   * Get the message queue handle by providing a message queue ID. Returns nullptr
   * if the queue ID is not contained inside the internal map.
   * @param messageQueueId
   * @return
   */
  MessageQueueIF* getMessageQueue(MessageQueueId_t messageQueueId) const;

 private:
  //! External instantiation is forbidden. Constructor still required for singleton instantiation.
  QueueMapManager();
  ~QueueMapManager();

  uint32_t queueCounter = MessageQueueIF::NO_QUEUE + 1;
  MutexIF* mapLock;
  QueueMap queueMap;
  static QueueMapManager* mqManagerInstance;
};

#endif /* FSFW_OSAL_HOST_QUEUEMAPMANAGER_H_ */
