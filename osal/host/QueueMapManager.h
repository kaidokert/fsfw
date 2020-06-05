#ifndef FRAMEWORK_OSAL_HOST_QUEUEMAP_H_
#define FRAMEWORK_OSAL_HOST_QUEUEMAP_H_

#include <framework/ipc/MessageQueueSenderIF.h>
#include <framework/osal/host/MessageQueue.h>
#include <unordered_map>
#include <atomic>

using QueueMap = std::unordered_map<MessageQueueId_t, MessageQueueIF*>;


/**
 * An internal map to map message queue IDs to message queues.
 * This propably should be a singleton..
 */
class QueueMapManager {
public:
	//! Returns the single instance of SemaphoreFactory.
	static QueueMapManager* instance();

	/**
	 * Insert a message queue into the map and returns a message queue ID
	 * @param queue The message queue to insert.
	 * @param id The passed value will be set unless a nullptr is passed
	 * @return
	 */
	ReturnValue_t addMessageQueue(MessageQueueIF* queue, MessageQueueId_t*
			id = nullptr);
	/**
	 * Get the message queue handle by providing a message queue ID.
	 * @param messageQueueId
	 * @return
	 */
	MessageQueueIF* getMessageQueue(MessageQueueId_t messageQueueId) const;

private:
	//! External instantiation is forbidden.
	QueueMapManager();
	std::atomic<uint32_t> queueCounter = MessageQueueIF::NO_QUEUE + 1;
	MutexIF* mapLock;
	QueueMap queueMap;
	static QueueMapManager* mqManagerInstance;
};



#endif /* FRAMEWORK_OSAL_HOST_QUEUEMAP_H_ */
