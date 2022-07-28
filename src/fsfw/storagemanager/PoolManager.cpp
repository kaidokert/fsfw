#include "fsfw/storagemanager/PoolManager.h"

#include "fsfw/FSFW.h"

PoolManager::PoolManager(object_id_t setObjectId, const LocalPoolConfig& localPoolConfig)
    : LocalPool(setObjectId, localPoolConfig, true) {
  mutex = MutexFactory::instance()->createMutex();
}

PoolManager::~PoolManager() { MutexFactory::instance()->deleteMutex(mutex); }

ReturnValue_t PoolManager::reserveSpace(const size_t size, store_address_t* address,
                                        bool ignoreFault) {
  MutexGuard mutexHelper(mutex, MutexIF::TimeoutType::WAITING, mutexTimeoutMs);
  ReturnValue_t status = LocalPool::reserveSpace(size, address, ignoreFault);
  return status;
}

ReturnValue_t PoolManager::deleteData(store_address_t storeId) {
#if FSFW_VERBOSE_LEVEL >= 2 && FSFW_OBJ_EVENT_TRANSLATION == 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "PoolManager( " << translateObject(getObjectId()) << " )::deleteData from store "
             << storeId.poolIndex << ". id is " << storeId.packetIndex << std::endl;
#endif
#endif
  MutexGuard mutexHelper(mutex, MutexIF::TimeoutType::WAITING, mutexTimeoutMs);
  return LocalPool::deleteData(storeId);
}

ReturnValue_t PoolManager::deleteData(uint8_t* buffer, size_t size, store_address_t* storeId) {
  MutexGuard mutexHelper(mutex, MutexIF::TimeoutType::WAITING, 20);
  ReturnValue_t status = LocalPool::deleteData(buffer, size, storeId);
  return status;
}

void PoolManager::setMutexTimeout(uint32_t mutexTimeoutMs) {
  this->mutexTimeoutMs = mutexTimeoutMs;
}

ReturnValue_t PoolManager::lockMutex(MutexIF::TimeoutType timeoutType, uint32_t timeoutMs) {
  return mutex->lockMutex(timeoutType, timeoutMs);
}

ReturnValue_t PoolManager::unlockMutex() { return mutex->unlockMutex(); }
