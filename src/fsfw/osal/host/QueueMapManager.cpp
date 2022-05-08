#include "fsfw/osal/host/QueueMapManager.h"

#include "fsfw/ipc/MutexFactory.h"
#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/serviceinterface.h"

QueueMapManager* QueueMapManager::mqManagerInstance = nullptr;

QueueMapManager::QueueMapManager() { mapLock = MutexFactory::instance()->createMutex(); }

QueueMapManager::~QueueMapManager() { MutexFactory::instance()->deleteMutex(mapLock); }

QueueMapManager* QueueMapManager::instance() {
  if (mqManagerInstance == nullptr) {
    mqManagerInstance = new QueueMapManager();
  }
  return QueueMapManager::mqManagerInstance;
}

ReturnValue_t QueueMapManager::addMessageQueue(MessageQueueIF* queueToInsert,
                                               MessageQueueId_t* id) {
  MutexGuard lock(mapLock);
  uint32_t currentId = queueCounter;
  queueCounter++;
  if (currentId == MessageQueueIF::NO_QUEUE) {
    // Skip the NO_QUEUE value
    currentId = queueCounter;
    queueCounter++;
  }
  auto returnPair = queueMap.emplace(currentId, queueToInsert);
  if (not returnPair.second) {
    /* This should never happen for the atomic variable. */
    FSFW_LOGE(
        "QueueMapManager::addMessageQueue The ID {} is already "
        "inside the map\n",
        currentId);
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  if (id != nullptr) {
    *id = currentId;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueIF* QueueMapManager::getMessageQueue(MessageQueueId_t messageQueueId) const {
  auto queueIter = queueMap.find(messageQueueId);
  if (queueIter != queueMap.end()) {
    return queueIter->second;
  } else {
    FSFW_LOGWT("getMessageQueue: The ID does not exists in the map\n");
  }
  return nullptr;
}
