/**
 *	@file	PoolManager
 *
 *  @date	02.02.2012
 *	@author	Bastian Baetz
 *
 *	@brief	This file contains the definition of the PoolManager class.
 */

#ifndef POOLMANAGER_H_
#define POOLMANAGER_H_


#include <framework/osal/OSAL.h>
#include <framework/storagemanager/LocalPool.h>

/**
 * @brief	The PoolManager class provides an intermediate data storage with
 * 			a fixed pool size policy for inter-process communication.
 * \details	Uses local pool, but is thread-safe.
 */

template <uint8_t NUMBER_OF_POOLS = 5>
class PoolManager : public LocalPool<NUMBER_OF_POOLS> {
protected:
	/**
	 * Overwritten for thread safety.
	 * Locks during execution.
	 */
	ReturnValue_t findEmpty( uint16_t pool_index, uint16_t* element );
	/**
	 * \brief	The mutex is created in the constructor and makes access mutual exclusive.
	 * \details	Locking and unlocking is done during searching for free slots and deleting existing slots.
	 */
	MutexId_t*	mutex;
public:
	PoolManager( object_id_t setObjectId, const uint16_t element_sizes[NUMBER_OF_POOLS], const uint16_t n_elements[NUMBER_OF_POOLS] );
	/**
	 * @brief	In the PoolManager's destructor all allocated memory is freed.
	 */
	virtual ~PoolManager( void );
	/**
	 * Overwritten for thread safety.
	 */
	virtual ReturnValue_t deleteData(store_address_t);
	virtual ReturnValue_t deleteData(uint8_t* buffer, uint32_t size, store_address_t* storeId = NULL);
};

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t PoolManager<NUMBER_OF_POOLS>::findEmpty(uint16_t pool_index,
		uint16_t* element) {
	ReturnValue_t mutexStatus = OSAL::lockMutex( mutex, OSAL::NO_TIMEOUT );
	ReturnValue_t status = this->DATA_STORAGE_FULL;
	if ( mutexStatus == this->RETURN_OK ) {
		status = LocalPool<NUMBER_OF_POOLS>::findEmpty(pool_index, element);
	} else {
		error << "PoolManager::findEmpty: Mutex could not be acquired. Error code: " << status << std::endl;
	}
	mutexStatus = OSAL::unlockMutex( mutex );
	if (mutexStatus != this->RETURN_OK) {
		return mutexStatus;
	} else {
		return status;
	}
}

template<uint8_t NUMBER_OF_POOLS>
inline PoolManager<NUMBER_OF_POOLS>::PoolManager(object_id_t setObjectId,
		const uint16_t element_sizes[NUMBER_OF_POOLS],
		const uint16_t n_elements[NUMBER_OF_POOLS]) : LocalPool<NUMBER_OF_POOLS>(setObjectId, element_sizes, n_elements, true) {
	mutex = new MutexId_t;
	ReturnValue_t result = OSAL::createMutex( OSAL::buildName('M','T','X','1'), ( mutex ) );
	if (result != this->RETURN_OK) {
		error << "PoolManager( " << std::hex << this->getObjectId() << std::dec << " )::ctor: Creating mutex failed." << std::endl;
	}
}

template<uint8_t NUMBER_OF_POOLS>
inline PoolManager<NUMBER_OF_POOLS>::~PoolManager(void) {
	OSAL::deleteMutex( mutex );
	delete mutex;
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t PoolManager<NUMBER_OF_POOLS>::deleteData(
		store_address_t packet_id) {
	//	debug << "PoolManager( " << translateObject(getObjectId()) << " )::deleteData from store " << packet_id.pool_index << ". id is " << packet_id.packet_index << std::endl;
		ReturnValue_t mutexStatus = OSAL::lockMutex( mutex, OSAL::NO_TIMEOUT );
		ReturnValue_t status = this->RETURN_OK;
		if ( mutexStatus == this->RETURN_OK ) {
			LocalPool<NUMBER_OF_POOLS>::deleteData(packet_id);
		} else {
			error << "PoolManager:deleteData: Mutex could not be acquired. Error code: " << status << std::endl;
		}
		mutexStatus = OSAL::unlockMutex( mutex );
		if (mutexStatus != this->RETURN_OK) {
			return mutexStatus;
		} else {
			return status;
		}
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t PoolManager<NUMBER_OF_POOLS>::deleteData(uint8_t* buffer, uint32_t size,
		store_address_t* storeId) {
	ReturnValue_t mutexStatus = OSAL::lockMutex( mutex, OSAL::NO_TIMEOUT );
	ReturnValue_t status = this->RETURN_OK;
	if ( mutexStatus == this->RETURN_OK ) {
		LocalPool<NUMBER_OF_POOLS>::deleteData(buffer, size, storeId);
	} else {
		error << "PoolManager:deleteData: Mutex could not be acquired. Error code: " << status << std::endl;
	}
	mutexStatus = OSAL::unlockMutex( mutex );
	if (mutexStatus != this->RETURN_OK) {
		return mutexStatus;
	} else {
		return status;
	}
}

#endif /* POOLMANAGER_H_ */
