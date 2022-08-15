#include "fsfw/osal/host/QueueMapManager.h"

#include "fsfw/ipc/MutexFactory.h"
#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

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

MessageQueueIF* QueueMapManager::getMessageQueue(MessageQueueId_t messageQueueId) const {
  auto queueIter = queueMap.find(messageQueueId);
  if (queueIter != queueMap.end()) {
    return queueIter->second;
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "QueueMapManager::getQueueHandle: The ID " << messageQueueId
                 << " does not exist in the map" << std::endl;
#else
    sif::printWarning("QueueMapManager::getQueueHandle: The ID %d does not exist in the map!\n",
                      messageQueueId);
#endif
  }
  return nullptr;
}
