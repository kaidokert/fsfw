#include "QueueMapManager.h"

#include "../../ipc/MutexFactory.h"
#include "../../ipc/MutexHelper.h"

QueueMapManager* QueueMapManager::mqManagerInstance = nullptr;

QueueMapManager::QueueMapManager() {
    mapLock = MutexFactory::instance()->createMutex();
}

QueueMapManager* QueueMapManager::instance() {
	if (mqManagerInstance == nullptr){
		mqManagerInstance = new QueueMapManager();
	}
	return QueueMapManager::mqManagerInstance;
}

ReturnValue_t QueueMapManager::addMessageQueue(
		MessageQueueIF* queueToInsert, MessageQueueId_t* id) {
	/* Not thread-safe, but it is assumed all message queues are created at software initialization
	now. If this is to be made thread-safe in the future, it propably would be sufficient to lock
	the increment operation here. */
	uint32_t currentId = queueCounter++;
	auto returnPair = queueMap.emplace(currentId, queueToInsert);
	if(not returnPair.second) {
		/* This should never happen for the atomic variable. */
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "QueueMapManager::addMessageQueue This ID is already "
		        "inside the map!" << std::endl;
#else
		sif::printError("QueueMapManager::addMessageQueue This ID is already "
                "inside the map!\n");
#endif
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	if (id != nullptr) {
		*id = currentId;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueIF* QueueMapManager::getMessageQueue(
		MessageQueueId_t messageQueueId) const {
	MutexHelper(mapLock, MutexIF::TimeoutType::WAITING, 50);
	auto queueIter = queueMap.find(messageQueueId);
	if(queueIter != queueMap.end()) {
		return queueIter->second;
	}
	else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::warning << "QueueMapManager::getQueueHandle: The ID " << messageQueueId <<
		        " does not exists in the map!" << std::endl;
#else
		sif::printWarning("QueueMapManager::getQueueHandle: The ID %d does not exist in the map!\n",
		        messageQueueId);
#endif
		return nullptr;
	}
}

