#include <framework/health/HealthTable.h>
#include <framework/serialize/SerializeAdapter.h>

HealthTable::HealthTable(object_id_t objectid) :
		SystemObject(objectid) {
	mutex = new MutexId_t;
	OSAL::createMutex(objectid + 1, mutex);

	mapIterator = healthMap.begin();
}

HealthTable::~HealthTable() {
	OSAL::deleteMutex(mutex);
	delete mutex;
}

ReturnValue_t HealthTable::registerObject(object_id_t object,
		HasHealthIF::HealthState initilialState) {
	if (healthMap.count(object) != 0) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	healthMap.insert(
			std::pair<object_id_t, HasHealthIF::HealthState>(object,
					initilialState));
	return HasReturnvaluesIF::RETURN_OK;
}

void HealthTable::setHealth(object_id_t object,
		HasHealthIF::HealthState newState) {
	OSAL::lockMutex(mutex, OSAL::NO_TIMEOUT);
	HealthMap::iterator iter = healthMap.find(object);
	if (iter != healthMap.end()) {
		iter->second = newState;
	}
	OSAL::unlockMutex(mutex);
}

HasHealthIF::HealthState HealthTable::getHealth(object_id_t object) {
	HasHealthIF::HealthState state = HasHealthIF::HEALTHY;
	OSAL::lockMutex(mutex, OSAL::NO_TIMEOUT);
	HealthMap::iterator iter = healthMap.find(object);
	if (iter != healthMap.end()) {
		state = iter->second;
	}
	OSAL::unlockMutex(mutex);
	return state;
}

uint32_t HealthTable::getPrintSize() {
	OSAL::lockMutex(mutex, OSAL::NO_TIMEOUT);
	uint32_t size = healthMap.size() * 5 + 2;
	OSAL::unlockMutex(mutex);
	return size;
}

bool HealthTable::hasHealth(object_id_t object) {
	bool exits = false;
	OSAL::lockMutex(mutex, OSAL::NO_TIMEOUT);
	HealthMap::iterator iter = healthMap.find(object);
	if (iter != healthMap.end()) {
		exits = true;
	}
	OSAL::unlockMutex(mutex);
	return exits;
}

void HealthTable::printAll(uint8_t* pointer, uint32_t maxSize) {
	OSAL::lockMutex(mutex, OSAL::NO_TIMEOUT);
	uint32_t size = 0;
	uint16_t count = healthMap.size();
	ReturnValue_t result = SerializeAdapter<uint16_t>::serialize(&count,
			&pointer, &size, maxSize, true);
	HealthMap::iterator iter;
	for (iter = healthMap.begin();
			iter != healthMap.end() && result == HasReturnvaluesIF::RETURN_OK;
			++iter) {
		result = SerializeAdapter<object_id_t>::serialize(&iter->first,
				&pointer, &size, maxSize, true);
		uint8_t health = iter->second;
		result = SerializeAdapter<uint8_t>::serialize(&health, &pointer, &size,
				maxSize, true);
	}
	OSAL::unlockMutex(mutex);
}

ReturnValue_t HealthTable::iterate(
		std::pair<object_id_t, HasHealthIF::HealthState> *value, bool reset) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	OSAL::lockMutex(mutex, OSAL::NO_TIMEOUT);
	if (reset) {
		mapIterator = healthMap.begin();
	}
	if (mapIterator == healthMap.end()) {
		result = HasReturnvaluesIF::RETURN_FAILED;
	}
	*value = *mapIterator;
	mapIterator++;
	OSAL::unlockMutex(mutex);

	return result;
}
