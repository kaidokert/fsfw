#ifndef FSFW_OSAL_FREERTOS_QUEUEMAPMANAGER_H_
#define FSFW_OSAL_FREERTOS_QUEUEMAPMANAGER_H_

#include <map>

#include "FreeRTOS.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/MutexIF.h"
#include "fsfw/ipc/messageQueueDefinitions.h"
#include "queue.h"

using QueueMap = std::map<MessageQueueId_t, QueueHandle_t>;

class QueueMapManager {
 public:
  //! Returns the single instance of QueueMapManager
  static QueueMapManager* instance();

  /**
   * Insert a message queue and the corresponding QueueHandle into the map
   * @param queue The message queue to insert.
   * @param id The passed value will be set unless a nullptr is passed
   * @return
   */
  ReturnValue_t addMessageQueue(QueueHandle_t queue, MessageQueueId_t* id);

  /**
   * Get the message queue handle by providing a message queue ID. Returns nullptr
   * if the queue ID does not exist in the internal map.
   * @param messageQueueId
   * @return
   */
  QueueHandle_t getMessageQueue(MessageQueueId_t messageQueueId) const;

 private:
  //! External instantiation forbidden. Constructor still required for singleton instantiation.
  QueueMapManager();
  ~QueueMapManager();

  uint32_t queueCounter = MessageQueueIF::NO_QUEUE + 1;
  MutexIF* mapLock;
  QueueMap queueMap;
  static QueueMapManager* mqManagerInstance;
};

#endif /* FSFW_OSAL_FREERTOS_QUEUEMAPMANAGER_H_ */
