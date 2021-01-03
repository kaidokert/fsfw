#include "InternalErrorReporter.h"

#include "../ipc/QueueFactory.h"
#include "../ipc/MutexFactory.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

InternalErrorReporter::InternalErrorReporter(object_id_t setObjectId,
        uint32_t messageQueueDepth): SystemObject(setObjectId),
		commandQueue(QueueFactory::instance()->
		        createMessageQueue(messageQueueDepth)),
		poolManager(this, commandQueue),
		internalErrorSid(setObjectId, InternalErrorDataset::ERROR_SET_ID),
		internalErrorDataset(this) {
	mutex = MutexFactory::instance()->createMutex();
}

InternalErrorReporter::~InternalErrorReporter() {
	MutexFactory::instance()->deleteMutex(mutex);
}

void InternalErrorReporter::setDiagnosticPrintout(bool enable) {
    this->diagnosticPrintout = enable;
}

ReturnValue_t InternalErrorReporter::performOperation(uint8_t opCode) {
    internalErrorDataset.read(INTERNAL_ERROR_MUTEX_TIMEOUT);

	uint32_t newQueueHits = getAndResetQueueHits();
	uint32_t newTmHits = getAndResetTmHits();
	uint32_t newStoreHits = getAndResetStoreHits();

#if FSFW_ENHANCED_PRINTOUT == 1
	if(diagnosticPrintout) {
	    if((newQueueHits > 0) or (newTmHits > 0) or (newStoreHits > 0)) {
#if CPP_OSTREAM_ENABLED == 1
	        sif::debug << "InternalErrorReporter::performOperation: Errors "
	                << "occured!" << std::endl;
	        sif::debug << "Queue errors: " << newQueueHits << std::endl;
	        sif::debug << "TM errors: " << newTmHits << std::endl;
	        sif::debug << "Store errors: " << newStoreHits << std::endl;
#endif
	    }
	}
#endif

	internalErrorDataset.queueHits.value += newQueueHits;
	internalErrorDataset.storeHits.value += newStoreHits;
	internalErrorDataset.tmHits.value += newTmHits;

	internalErrorDataset.commit(INTERNAL_ERROR_MUTEX_TIMEOUT);

	poolManager.performHkOperation();

	CommandMessage message;
	ReturnValue_t result = commandQueue->receiveMessage(&message);
	if(result != MessageQueueIF::EMPTY) {
	    poolManager.handleHousekeepingMessage(&message);
	}
	return HasReturnvaluesIF::RETURN_OK;
}

void InternalErrorReporter::queueMessageNotSent() {
	incrementQueueHits();
}

void InternalErrorReporter::lostTm() {
	incrementTmHits();
}

uint32_t InternalErrorReporter::getAndResetQueueHits() {
	uint32_t value;
	mutex->lockMutex(MutexIF::WAITING, INTERNAL_ERROR_MUTEX_TIMEOUT);
	value = queueHits;
	queueHits = 0;
	mutex->unlockMutex();
	return value;
}

uint32_t InternalErrorReporter::getQueueHits() {
	uint32_t value;
	mutex->lockMutex(MutexIF::WAITING, INTERNAL_ERROR_MUTEX_TIMEOUT);
	value = queueHits;
	mutex->unlockMutex();
	return value;
}

void InternalErrorReporter::incrementQueueHits() {
	mutex->lockMutex(MutexIF::WAITING, INTERNAL_ERROR_MUTEX_TIMEOUT);
	queueHits++;
	mutex->unlockMutex();
}

uint32_t InternalErrorReporter::getAndResetTmHits() {
	uint32_t value;
	mutex->lockMutex(MutexIF::WAITING, INTERNAL_ERROR_MUTEX_TIMEOUT);
	value = tmHits;
	tmHits = 0;
	mutex->unlockMutex();
	return value;
}

uint32_t InternalErrorReporter::getTmHits() {
	uint32_t value;
	mutex->lockMutex(MutexIF::WAITING, INTERNAL_ERROR_MUTEX_TIMEOUT);
	value = tmHits;
	mutex->unlockMutex();
	return value;
}

void InternalErrorReporter::incrementTmHits() {
	mutex->lockMutex(MutexIF::WAITING, INTERNAL_ERROR_MUTEX_TIMEOUT);
	tmHits++;
	mutex->unlockMutex();
}

void InternalErrorReporter::storeFull() {
	incrementStoreHits();
}

uint32_t InternalErrorReporter::getAndResetStoreHits() {
	uint32_t value;
	mutex->lockMutex(MutexIF::WAITING, INTERNAL_ERROR_MUTEX_TIMEOUT);
	value = storeHits;
	storeHits = 0;
	mutex->unlockMutex();
	return value;
}

uint32_t InternalErrorReporter::getStoreHits() {
	uint32_t value;
	mutex->lockMutex(MutexIF::WAITING, INTERNAL_ERROR_MUTEX_TIMEOUT);
	value = storeHits;
	mutex->unlockMutex();
	return value;
}

void InternalErrorReporter::incrementStoreHits() {
	mutex->lockMutex(MutexIF::WAITING, INTERNAL_ERROR_MUTEX_TIMEOUT);
	storeHits++;
	mutex->unlockMutex();
}

object_id_t InternalErrorReporter::getObjectId() const {
    return SystemObject::getObjectId();
}

MessageQueueId_t InternalErrorReporter::getCommandQueue() const {
    return this->commandQueue->getId();
}

ReturnValue_t InternalErrorReporter::initializeLocalDataPool(
        LocalDataPool &localDataPoolMap, LocalDataPoolManager &poolManager) {
    localDataPoolMap.emplace(errorPoolIds::TM_HITS,
            new PoolEntry<uint32_t>());
    localDataPoolMap.emplace(errorPoolIds::QUEUE_HITS,
            new PoolEntry<uint32_t>());
    localDataPoolMap.emplace(errorPoolIds::STORE_HITS,
            new PoolEntry<uint32_t>());
    poolManager.subscribeForPeriodicPacket(internalErrorSid, false,
            getPeriodicOperationFrequency(), true);
    internalErrorDataset.setValidity(true, true);
    return HasReturnvaluesIF::RETURN_OK;
}

LocalDataPoolManager* InternalErrorReporter::getHkManagerHandle() {
    return &poolManager;
}

dur_millis_t InternalErrorReporter::getPeriodicOperationFrequency() const {
    return this->executingTask->getPeriodMs();
}

LocalPoolDataSetBase* InternalErrorReporter::getDataSetHandle(sid_t sid) {
    return &internalErrorDataset;
}

void InternalErrorReporter::setTaskIF(PeriodicTaskIF *task) {
    this->executingTask = task;
}

ReturnValue_t InternalErrorReporter::initialize() {
    ReturnValue_t result = poolManager.initialize(commandQueue);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    return SystemObject::initialize();
}

ReturnValue_t InternalErrorReporter::initializeAfterTaskCreation() {
    return poolManager.initializeAfterTaskCreation();
}

