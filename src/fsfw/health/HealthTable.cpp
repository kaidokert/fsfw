#include "fsfw/health/HealthTable.h"

#include "fsfw/ipc/MutexFactory.h"
#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/serialize/SerializeAdapter.h"

HealthTable::HealthTable(object_id_t objectid) : SystemObject(objectid) {
  mutex = MutexFactory::instance()->createMutex();
  ;

  mapIterator = healthMap.begin();
}

void HealthTable::setMutexTimeout(MutexIF::TimeoutType timeoutType, uint32_t timeoutMs) {
  this->timeoutType = timeoutType;
  this->mutexTimeoutMs = timeoutMs;
}

HealthTable::~HealthTable() { MutexFactory::instance()->deleteMutex(mutex); }

ReturnValue_t HealthTable::registerObject(object_id_t object,
                                          HasHealthIF::HealthState initilialState) {
  if (healthMap.count(object) != 0) {
    return returnvalue::FAILED;
  }
  healthMap.emplace(object, initilialState);
  return returnvalue::OK;
}

ReturnValue_t HealthTable::removeObject(object_id_t object) {
  mapIterator = healthMap.find(object);
  if (mapIterator == healthMap.end()) {
    return returnvalue::FAILED;
  }
  healthMap.erase(mapIterator);
  return returnvalue::OK;
}

void HealthTable::setHealth(object_id_t object, HasHealthIF::HealthState newState) {
  MutexGuard(mutex, timeoutType, mutexTimeoutMs);
  HealthMap::iterator iter = healthMap.find(object);
  if (iter != healthMap.end()) {
    iter->second = newState;
  }
}

HasHealthIF::HealthState HealthTable::getHealth(object_id_t object) {
  HasHealthIF::HealthState state = HasHealthIF::HEALTHY;
  MutexGuard(mutex, timeoutType, mutexTimeoutMs);
  HealthMap::iterator iter = healthMap.find(object);
  if (iter != healthMap.end()) {
    state = iter->second;
  }
  return state;
}

bool HealthTable::hasHealth(object_id_t object) {
  MutexGuard(mutex, timeoutType, mutexTimeoutMs);
  HealthMap::iterator iter = healthMap.find(object);
  if (iter != healthMap.end()) {
    return true;
  }
  return false;
}

size_t HealthTable::getPrintSize() {
  MutexGuard(mutex, timeoutType, mutexTimeoutMs);
  uint32_t size =
      healthMap.size() * sizeof(object_id_t) + sizeof(HasHealthIF::HealthState) + sizeof(uint16_t);
  return size;
}

void HealthTable::printAll(uint8_t* pointer, size_t maxSize) {
  MutexGuard(mutex, timeoutType, mutexTimeoutMs);
  size_t size = 0;
  uint16_t count = healthMap.size();
  ReturnValue_t result =
      SerializeAdapter::serialize(&count, &pointer, &size, maxSize, SerializeIF::Endianness::BIG);
  if (result != returnvalue::OK) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "HealthTable::printAll: Serialization of health table failed" << std::endl;
#else
    sif::printWarning("HealthTable::printAll: Serialization of health table failed\n");
#endif
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    return;
  }
  for (const auto& health : healthMap) {
    result = SerializeAdapter::serialize(&health.first, &pointer, &size, maxSize,
                                         SerializeIF::Endianness::BIG);
    if (result != returnvalue::OK) {
      return;
    }
    uint8_t healthValue = health.second;
    result = SerializeAdapter::serialize(&healthValue, &pointer, &size, maxSize,
                                         SerializeIF::Endianness::BIG);
    if (result != returnvalue::OK) {
      return;
    }
  }
}

ReturnValue_t HealthTable::iterate(HealthEntry* value, bool reset) {
  ReturnValue_t result = returnvalue::OK;
  MutexGuard(mutex, timeoutType, mutexTimeoutMs);
  if (reset) {
    mapIterator = healthMap.begin();
  }
  if (mapIterator == healthMap.end()) {
    return returnvalue::FAILED;
  }
  *value = *mapIterator;
  mapIterator++;
  return result;
}
