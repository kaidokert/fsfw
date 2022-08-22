#include "fsfw/osal/freertos/QueueMapManager.h"

#include "fsfw/ipc/MutexFactory.h"
#include "fsfw/ipc/MutexGuard.h"

QueueMapManager* QueueMapManager::mqManagerInstance = nullptr;

QueueMapManager::QueueMapManager() { mapLock = MutexFactory::instance()->createMutex(); }

QueueMapManager* QueueMapManager::instance() {
  if (mqManagerInstance == nullptr) {
    mqManagerInstance = new QueueMapManager();
  }
  return QueueMapManager::mqManagerInstance;
}

ReturnValue_t QueueMapManager::addMessageQueue(QueueHandle_t queue, MessageQueueId_t* id) {
  MutexGuard lock(mapLock);
  uint32_t currentId = queueCounter;
  queueCounter++;
  if (currentId == MessageQueueIF::NO_QUEUE) {
    // Skip the NO_QUEUE value
    currentId = queueCounter;
    queueCounter++;
  }
  auto returnPair = queueMap.emplace(currentId, queue);
  if (not returnPair.second) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "QueueMapManager::addMessageQueue This ID is already "
                  "inside the map!"
               << std::endl;
#else
    sif::printError(
        "QueueMapManager::addMessageQueue This ID is already "
        "inside the map!\n");
#endif
    return returnvalue::FAILED;
  }
  if (id != nullptr) {
    *id = currentId;
  }
  return returnvalue::OK;
}

QueueHandle_t QueueMapManager::getMessageQueue(MessageQueueId_t messageQueueId) const {
  auto queueIter = queueMap.find(messageQueueId);
  if (queueIter != queueMap.end()) {
    return queueIter->second;
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "QueueMapManager::getQueueHandle: The ID " << messageQueueId
                 << " does not exists in the map!" << std::endl;
#else
    sif::printWarning("QueueMapManager::getQueueHandle: The ID %d does not exist in the map!\n",
                      messageQueueId);
#endif
  }
  return nullptr;
}

QueueMapManager::~QueueMapManager() { MutexFactory::instance()->deleteMutex(mapLock); }
