#ifndef FSFW_STORAGEMANAGER_POOLMANAGER_H_
#define FSFW_STORAGEMANAGER_POOLMANAGER_H_

#include "LocalPool.h"
#include "StorageAccessor.h"
#include "../ipc/MutexHelper.h"


/**
 * @brief	The PoolManager class provides an intermediate data storage with
 * 			a fixed pool size policy for inter-process communication.
 * @details	Uses local pool calls but is thread safe by protecting the call
 * 			with a lock.
 * @author 	Bastian Baetz
 */
template <uint8_t NUMBER_OF_POOLS = 5>
class PoolManager : public LocalPool<NUMBER_OF_POOLS> {
public:
	PoolManager(object_id_t setObjectId,
			const uint16_t element_sizes[NUMBER_OF_POOLS],
			const uint16_t n_elements[NUMBER_OF_POOLS]);

	/**
	 * @brief	In the PoolManager's destructor all allocated memory
	 * 			is freed.
	 */
	virtual ~PoolManager();

	/**
	 * @brief 	LocalPool overrides for thread-safety. Decorator function
	 * 			which wraps LocalPool calls with a mutex protection.
	 */
	ReturnValue_t deleteData(store_address_t) override;
	ReturnValue_t deleteData(uint8_t* buffer, size_t size,
			store_address_t* storeId = nullptr) override;

	void setMutexTimeout(uint32_t mutexTimeoutMs);
protected:
	//! Default mutex timeout value to prevent permanent blocking.
	uint32_t mutexTimeoutMs = 20;

	ReturnValue_t reserveSpace(const uint32_t size, store_address_t* address,
			bool ignoreFault) override;

	/**
	 * @brief	The mutex is created in the constructor and makes
	 * 			access mutual exclusive.
	 * @details	Locking and unlocking is done during searching for free slots
	 * 			and deleting existing slots.
	 */
	MutexIF* mutex;
};

#include "PoolManager.tpp"

#endif /* FSFW_STORAGEMANAGER_POOLMANAGER_H_ */
