/*
 * DataPool.cpp
 *
 *  Created on: 17.10.2012
 *      Author: baetz
 */


#include <framework/datapool/DataPool.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
DataPool::DataPool( void ( *initFunction )( std::map<uint32_t, PoolEntryIF*>* pool_map ) ) {
	this->mutex = new MutexId_t;
	OSAL::createMutex( OSAL::buildName('M','T','X','0'), ( this->mutex ) );
	if (initFunction != NULL ) {
		initFunction( &this->data_pool );
	}
}

DataPool::~DataPool() {
	OSAL::deleteMutex( this->mutex );
	delete this->mutex;
	for ( std::map<uint32_t, PoolEntryIF*>::iterator it = this->data_pool.begin(); it != this->data_pool.end(); ++it ) {
		delete it->second;
	}
}

//The function checks PID, type and array length before returning a copy of the PoolEntry. In failure case, it returns a temp-Entry with size 0 and NULL-ptr.
template <typename T> PoolEntry<T>* DataPool::getData( uint32_t data_pool_id,  uint8_t sizeOrPosition ) {
	std::map<uint32_t, PoolEntryIF*>::iterator it = this->data_pool.find( data_pool_id );
	if ( it != this->data_pool.end() ) {
		PoolEntry<T>* entry = dynamic_cast< PoolEntry<T>* >( it->second );
		if (entry != NULL ) {
			if ( sizeOrPosition <= entry->length ) {
				return entry;
			}
		}
	}
	return NULL;
}

PoolEntryIF* DataPool::getRawData( uint32_t data_pool_id ) {
	std::map<uint32_t, PoolEntryIF*>::iterator it = this->data_pool.find( data_pool_id );
	if ( it != this->data_pool.end() ) {
		return it->second;
	} else {
		return NULL;
	}
}

//uint8_t DataPool::getRawData( uint32_t data_pool_id, uint8_t* address, uint16_t* size, uint32_t max_size ) {
//	std::map<uint32_t, PoolEntryIF*>::iterator it = this->data_pool.find( data_pool_id );
//	if ( it != this->data_pool.end() ) {
//		if ( it->second->getByteSize() <= max_size ) {
//			*size = it->second->getByteSize();
//			memcpy( address, it->second->getRawData(), *size );
//			return DP_SUCCESSFUL;
//		}
//	}
//	*size = 0;
//	return DP_FAILURE;
//}

ReturnValue_t DataPool::freeDataPoolLock() {
	ReturnValue_t status = OSAL::unlockMutex( this->mutex );
	if ( status != RETURN_OK ) {
		error << "DataPool::DataPool: unlock of mutex failed with error code: " << status << std::endl;
	}
	return status;
}

ReturnValue_t DataPool::lockDataPool() {
	ReturnValue_t status = OSAL::lockMutex( this->mutex, OSAL::NO_TIMEOUT );
	if ( status != RETURN_OK ) {
		error << "DataPool::DataPool: lock of mutex failed with error code: " << status << std::endl;
	}
	return status;
}

void DataPool::print() {
	debug << "DataPool contains: " << std::endl;
	std::map<uint32_t, PoolEntryIF*>::iterator	dataPoolIt;
	dataPoolIt = this->data_pool.begin();
	while( dataPoolIt != this->data_pool.end() ) {
		debug << std::hex << dataPoolIt->first << std::dec << " |";
		dataPoolIt->second->print();
		dataPoolIt++;
	}
}

template PoolEntry<uint8_t>* DataPool::getData<uint8_t>( uint32_t data_pool_id,  uint8_t size );
template PoolEntry<uint16_t>* DataPool::getData<uint16_t>( uint32_t data_pool_id,  uint8_t size );
template PoolEntry<uint32_t>* DataPool::getData<uint32_t>( uint32_t data_pool_id,  uint8_t size );
template PoolEntry<uint64_t>* DataPool::getData<uint64_t>(uint32_t data_pool_id,
		uint8_t size);
template PoolEntry<int8_t>* DataPool::getData<int8_t>( uint32_t data_pool_id,  uint8_t size );
template PoolEntry<int16_t>* DataPool::getData<int16_t>( uint32_t data_pool_id,  uint8_t size );
template PoolEntry<int32_t>* DataPool::getData<int32_t>( uint32_t data_pool_id,  uint8_t size );
template PoolEntry<float>* DataPool::getData<float>( uint32_t data_pool_id,  uint8_t size );
template PoolEntry<double>* DataPool::getData<double>(uint32_t data_pool_id,
		uint8_t size);


uint32_t DataPool::PIDToDataPoolId(uint32_t parameter_id) {
	return (parameter_id >> 8) & 0x00FFFFFF;
}

uint8_t DataPool::PIDToArrayIndex(uint32_t parameter_id) {
	return (parameter_id & 0x000000FF);
}

uint32_t DataPool::poolIdAndPositionToPid(uint32_t poolId, uint8_t index) {
	return (poolId << 8) + index;
}

//TODO: This is not 100% clean. Should return returnValue and type by passing...
//TODO: Do we need a mutex lock here... I don't think so, as we only check static const values of elements in a list that do not change.
Type DataPool::getType(uint32_t parameter_id) {
	std::map<uint32_t, PoolEntryIF*>::iterator it = this->data_pool.find( PIDToDataPoolId(parameter_id));
	if ( it != this->data_pool.end() ) {
		return it->second->getType();
	} else {
		return Type::UNKNOWN_TYPE;
	}
}

bool DataPool::exists(uint32_t parameterId) {
	uint32_t poolId = PIDToDataPoolId(parameterId);
	uint32_t index = PIDToArrayIndex(parameterId);
	std::map<uint32_t, PoolEntryIF*>::iterator it = this->data_pool.find( poolId );
	if (it != data_pool.end()) {
		if (it->second->getSize() >= index) {
			return true;
		}
	}
	return false;
}
