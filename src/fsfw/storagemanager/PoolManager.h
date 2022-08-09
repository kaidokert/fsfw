#ifndef FSFW_STORAGEMANAGER_POOLMANAGER_H_
#define FSFW_STORAGEMANAGER_POOLMANAGER_H_

#include "../ipc/MutexGuard.h"
#include "LocalPool.h"
#include "StorageAccessor.h"

/**
 * @brief	The PoolManager class provides an intermediate data storage with
 * 			a fixed pool size policy for inter-process communication.
 * @details
 * Uses local pool calls but is thread safe by protecting most calls
 * with a lock. The developer can lock the pool with the provided API
 * if the lock needs to persists beyond the function call.
 *
 * Other than that, the class provides the same interface as the LocalPool
 * class. The class is always registered as a system object as it is assumed
 * it will always be used concurrently (if this is not the case, it is
 * recommended to use the LocalPool class instead).
 * @author 	Bastian Baetz
 */
class PoolManager : public LocalPool {
 public:
  PoolManager(object_id_t setObjectId, const LocalPoolConfig& poolConfig);

  /**
   * @brief	In the PoolManager's destructor all allocated memory
   * 			is freed.
   */
  ~PoolManager() override;

  /**
   * Set the default mutex timeout for internal calls.
   * @param mutexTimeoutMs
   */
  void setMutexTimeout(uint32_t mutexTimeoutMs);

  /**
   * @brief 	LocalPool overrides for thread-safety. Decorator function
   * 			which wraps LocalPool calls with a mutex protection.
   */
  ReturnValue_t deleteData(store_address_t) override;
  ReturnValue_t deleteData(uint8_t* buffer, size_t size, store_address_t* storeId) override;

  /**
   * The developer is allowed to lock the mutex in case the lock needs
   * to persist beyond the function calls which are not protected by the
   * class.
   * @param timeoutType
   * @param timeoutMs
   * @return
   */
  ReturnValue_t lockMutex(MutexIF::TimeoutType timeoutType, uint32_t timeoutMs);
  ReturnValue_t unlockMutex();

 protected:
  //! Default mutex timeout value to prevent permanent blocking.
  uint32_t mutexTimeoutMs = 20;

  ReturnValue_t reserveSpace(size_t size, store_address_t* address, bool ignoreFault) override;

  /**
   * @brief	The mutex is created in the constructor and makes
   * 			access mutual exclusive.
   * @details	Locking and unlocking is done during searching for free slots
   * 			and deleting existing slots.
   */
  MutexIF* mutex;
};

#endif /* FSFW_STORAGEMANAGER_POOLMANAGER_H_ */
