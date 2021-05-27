#ifndef FSFW_OSAL_FREERTOS_QUEUEMAPMANAGER_H_
#define FSFW_OSAL_FREERTOS_QUEUEMAPMANAGER_H_

#include "../../ipc/MutexIF.h"
#include "../../ipc/messageQueueDefinitions.h"
#include "../../ipc/MessageQueueIF.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include <map>

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

    uint32_t queueCounter = 0;
    MutexIF* mapLock;
    QueueMap queueMap;
    static QueueMapManager* mqManagerInstance;
};



#endif /* FSFW_OSAL_FREERTOS_QUEUEMAPMANAGER_H_ */
