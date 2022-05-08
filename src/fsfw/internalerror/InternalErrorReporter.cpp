#include "fsfw/internalerror/InternalErrorReporter.h"

#include "fsfw/datapool/PoolReadGuard.h"
#include "fsfw/ipc/MutexFactory.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/serviceinterface.h"

InternalErrorReporter::InternalErrorReporter(object_id_t setObjectId, uint32_t messageQueueDepth)
    : SystemObject(setObjectId),
      commandQueue(QueueFactory::instance()->createMessageQueue(messageQueueDepth)),
      poolManager(this, commandQueue),
      internalErrorSid(setObjectId, InternalErrorDataset::ERROR_SET_ID),
      internalErrorDataset(this) {
  mutex = MutexFactory::instance()->createMutex();
}

InternalErrorReporter::~InternalErrorReporter() { MutexFactory::instance()->deleteMutex(mutex); }

void InternalErrorReporter::setDiagnosticPrintout(bool enable) {
  this->diagnosticPrintout = enable;
}

ReturnValue_t InternalErrorReporter::performOperation(uint8_t opCode) {
  CommandMessage message;
  ReturnValue_t result = commandQueue->receiveMessage(&message);
  if (result != MessageQueueIF::EMPTY) {
    poolManager.handleHousekeepingMessage(&message);
  }

  uint32_t newQueueHits = getAndResetQueueHits();
  uint32_t newTmHits = getAndResetTmHits();
  uint32_t newStoreHits = getAndResetStoreHits();

#if FSFW_VERBOSE_LEVEL >= 1
  if (diagnosticPrintout) {
    if ((newQueueHits > 0) or (newTmHits > 0) or (newStoreHits > 0)) {
      FSFW_LOGW("performOperation: Errors occured\nQueue {} | TM {} | Store {}\n", newQueueHits,
                newTmHits, newStoreHits);
    }
  }
#endif

  {
    PoolReadGuard readGuard(&internalErrorDataset);
    if (readGuard.getReadResult() == HasReturnvaluesIF::RETURN_OK) {
      internalErrorDataset.queueHits.value += newQueueHits;
      internalErrorDataset.storeHits.value += newStoreHits;
      internalErrorDataset.tmHits.value += newTmHits;
      internalErrorDataset.setValidity(true, true);
      if ((newQueueHits != 0) or (newStoreHits != 0) or (newTmHits != 0)) {
        internalErrorDataset.setChanged(true);
      }
    }
  }

  poolManager.performHkOperation();
  return HasReturnvaluesIF::RETURN_OK;
}

void InternalErrorReporter::queueMessageNotSent() { incrementQueueHits(); }

void InternalErrorReporter::lostTm() { incrementTmHits(); }

uint32_t InternalErrorReporter::getAndResetQueueHits() {
  uint32_t value;
  mutex->lockMutex(timeoutType, timeoutMs);
  value = queueHits;
  queueHits = 0;
  mutex->unlockMutex();
  return value;
}

void InternalErrorReporter::incrementQueueHits() {
  mutex->lockMutex(timeoutType, timeoutMs);
  queueHits++;
  mutex->unlockMutex();
}

uint32_t InternalErrorReporter::getAndResetTmHits() {
  uint32_t value;
  mutex->lockMutex(timeoutType, timeoutMs);
  value = tmHits;
  tmHits = 0;
  mutex->unlockMutex();
  return value;
}

void InternalErrorReporter::incrementTmHits() {
  mutex->lockMutex(timeoutType, timeoutMs);
  tmHits++;
  mutex->unlockMutex();
}

void InternalErrorReporter::storeFull() { incrementStoreHits(); }

uint32_t InternalErrorReporter::getAndResetStoreHits() {
  uint32_t value;
  mutex->lockMutex(timeoutType, timeoutMs);
  value = storeHits;
  storeHits = 0;
  mutex->unlockMutex();
  return value;
}

void InternalErrorReporter::incrementStoreHits() {
  mutex->lockMutex(timeoutType, timeoutMs);
  storeHits++;
  mutex->unlockMutex();
}

object_id_t InternalErrorReporter::getObjectId() const { return SystemObject::getObjectId(); }

MessageQueueId_t InternalErrorReporter::getCommandQueue() const {
  return this->commandQueue->getId();
}

ReturnValue_t InternalErrorReporter::initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
                                                             LocalDataPoolManager &poolManager) {
  localDataPoolMap.emplace(errorPoolIds::TM_HITS, new PoolEntry<uint32_t>());
  localDataPoolMap.emplace(errorPoolIds::QUEUE_HITS, new PoolEntry<uint32_t>());
  localDataPoolMap.emplace(errorPoolIds::STORE_HITS, new PoolEntry<uint32_t>());
  poolManager.subscribeForPeriodicPacket(internalErrorSid, false, getPeriodicOperationFrequency(),
                                         true);
  internalErrorDataset.setValidity(true, true);
  return HasReturnvaluesIF::RETURN_OK;
}

dur_millis_t InternalErrorReporter::getPeriodicOperationFrequency() const {
  return this->executingTask->getPeriodMs();
}

LocalPoolDataSetBase *InternalErrorReporter::getDataSetHandle(sid_t sid) {
  return &internalErrorDataset;
}

void InternalErrorReporter::setTaskIF(PeriodicTaskIF *task) { this->executingTask = task; }

ReturnValue_t InternalErrorReporter::initialize() {
  ReturnValue_t result = poolManager.initialize(commandQueue);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return SystemObject::initialize();
}

ReturnValue_t InternalErrorReporter::initializeAfterTaskCreation() {
  return poolManager.initializeAfterTaskCreation();
}

void InternalErrorReporter::setMutexTimeout(MutexIF::TimeoutType timeoutType, uint32_t timeoutMs) {
  this->timeoutType = timeoutType;
  this->timeoutMs = timeoutMs;
}

LocalDataPoolManager *InternalErrorReporter::getHkManagerHandle() { return &poolManager; }
