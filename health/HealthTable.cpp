#include "HealthTable.h"
#include "../ipc/MutexHelper.h"
#include "../ipc/MutexFactory.h"
#include "../serialize/SerializeAdapter.h"

HealthTable::HealthTable(object_id_t objectid) :
		SystemObject(objectid) {
	mutex = MutexFactory::instance()->createMutex();;

	mapIterator = healthMap.begin();
}

void HealthTable::setMutexTimeout(MutexIF::TimeoutType timeoutType,
		uint32_t timeoutMs) {
	this->timeoutType = timeoutType;
	this->mutexTimeoutMs = timeoutMs;
}

HealthTable::~HealthTable() {
	MutexFactory::instance()->deleteMutex(mutex);
}

ReturnValue_t HealthTable::registerObject(object_id_t object,
		HasHealthIF::HealthState initilialState) {
	if (healthMap.count(object) != 0) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	healthMap.emplace(object, initilialState);
	return HasReturnvaluesIF::RETURN_OK;
}

void HealthTable::setHealth(object_id_t object,
		HasHealthIF::HealthState newState) {
    MutexHelper(mutex, timeoutType, mutexTimeoutMs);
	HealthMap::iterator iter = healthMap.find(object);
	if (iter != healthMap.end()) {
		iter->second = newState;
	}
}

HasHealthIF::HealthState HealthTable::getHealth(object_id_t object) {
	HasHealthIF::HealthState state = HasHealthIF::HEALTHY;
	MutexHelper(mutex, timeoutType, mutexTimeoutMs);
	HealthMap::iterator iter = healthMap.find(object);
	if (iter != healthMap.end()) {
		state = iter->second;
	}
	return state;
}

bool HealthTable::hasHealth(object_id_t object) {
	MutexHelper(mutex, timeoutType, mutexTimeoutMs);
	HealthMap::iterator iter = healthMap.find(object);
	if (iter != healthMap.end()) {
		return true;
	}
	return false;
}

size_t HealthTable::getPrintSize() {
    MutexHelper(mutex, timeoutType, mutexTimeoutMs);
    uint32_t size = healthMap.size() * sizeof(object_id_t) +
            sizeof(HasHealthIF::HealthState) + sizeof(uint16_t);
    return size;
}

void HealthTable::printAll(uint8_t* pointer, size_t maxSize) {
    MutexHelper(mutex, timeoutType, mutexTimeoutMs);
	size_t size = 0;
	uint16_t count = healthMap.size();
	SerializeAdapter::serialize(&count,
			&pointer, &size, maxSize, SerializeIF::Endianness::BIG);
	for (const auto& health: healthMap) {
		SerializeAdapter::serialize(&health.first,
				&pointer, &size, maxSize, SerializeIF::Endianness::BIG);
		uint8_t healthValue = health.second;
		SerializeAdapter::serialize(&healthValue, &pointer, &size,
				maxSize, SerializeIF::Endianness::BIG);
	}
}

ReturnValue_t HealthTable::iterate(HealthEntry *value, bool reset) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	MutexHelper(mutex, timeoutType, mutexTimeoutMs);
	if (reset) {
		mapIterator = healthMap.begin();
	}
	if (mapIterator == healthMap.end()) {
		result = HasReturnvaluesIF::RETURN_FAILED;
	}
	*value = *mapIterator;
	mapIterator++;
	return result;
}
