#include "InternalErrorReporter.h"

#include "../datapool/DataSet.h"
#include "../datapool/PoolVariable.h"
#include "../ipc/MutexFactory.h"

#include "../serviceinterface/ServiceInterfaceStream.h"

InternalErrorReporter::InternalErrorReporter(object_id_t setObjectId,
		uint32_t queuePoolId, uint32_t tmPoolId, uint32_t storePoolId) :
		SystemObject(setObjectId), mutex(NULL), queuePoolId(queuePoolId), tmPoolId(
				tmPoolId), storePoolId(
				storePoolId), queueHits(0), tmHits(0), storeHits(
				0) {
	mutex = MutexFactory::instance()->createMutex();
}

InternalErrorReporter::~InternalErrorReporter() {
	MutexFactory::instance()->deleteMutex(mutex);
}

ReturnValue_t InternalErrorReporter::performOperation(uint8_t opCode) {

	DataSet mySet;
	PoolVariable<uint32_t> queueHitsInPool(queuePoolId, &mySet,
			PoolVariableIF::VAR_READ_WRITE);
	PoolVariable<uint32_t> tmHitsInPool(tmPoolId, &mySet,
			PoolVariableIF::VAR_READ_WRITE);

	PoolVariable<uint32_t> storeHitsInPool(storePoolId, &mySet,
			PoolVariableIF::VAR_READ_WRITE);
	mySet.read();

	uint32_t newQueueHits = getAndResetQueueHits();
	uint32_t newTmHits = getAndResetTmHits();
	uint32_t newStoreHits = getAndResetStoreHits();

	queueHitsInPool.value += newQueueHits;
	tmHitsInPool.value += newTmHits;
	storeHitsInPool.value += newStoreHits;

	mySet.commit(PoolVariableIF::VALID);

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
	mutex->lockMutex(MutexIF::BLOCKING);
	value = queueHits;
	queueHits = 0;
	mutex->unlockMutex();
	return value;
}

uint32_t InternalErrorReporter::getQueueHits() {
	uint32_t value;
	mutex->lockMutex(MutexIF::BLOCKING);
	value = queueHits;
	mutex->unlockMutex();
	return value;
}

void InternalErrorReporter::incrementQueueHits() {
	mutex->lockMutex(MutexIF::BLOCKING);
	queueHits++;
	mutex->unlockMutex();
}

uint32_t InternalErrorReporter::getAndResetTmHits() {
	uint32_t value;
	mutex->lockMutex(MutexIF::BLOCKING);
	value = tmHits;
	tmHits = 0;
	mutex->unlockMutex();
	return value;
}

uint32_t InternalErrorReporter::getTmHits() {
	uint32_t value;
	mutex->lockMutex(MutexIF::BLOCKING);
	value = tmHits;
	mutex->unlockMutex();
	return value;
}

void InternalErrorReporter::incrementTmHits() {
	mutex->lockMutex(MutexIF::BLOCKING);
	tmHits++;
	mutex->unlockMutex();
}

void InternalErrorReporter::storeFull() {
	incrementStoreHits();
}

uint32_t InternalErrorReporter::getAndResetStoreHits() {
	uint32_t value;
	mutex->lockMutex(MutexIF::BLOCKING);
	value = storeHits;
	storeHits = 0;
	mutex->unlockMutex();
	return value;
}

uint32_t InternalErrorReporter::getStoreHits() {
	uint32_t value;
	mutex->lockMutex(MutexIF::BLOCKING);
	value = storeHits;
	mutex->unlockMutex();
	return value;
}

void InternalErrorReporter::incrementStoreHits() {
	mutex->lockMutex(MutexIF::BLOCKING);
	storeHits++;
	mutex->unlockMutex();
}
