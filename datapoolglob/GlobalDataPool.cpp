#include <framework/datapoolglob/GlobalDataPool.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/ipc/MutexFactory.h>

GlobalDataPool::GlobalDataPool(
		void(*initFunction)(GlobPoolMap* pool_map)) {
	mutex = MutexFactory::instance()->createMutex();
	if (initFunction != NULL ) {
		initFunction( &this->globDataPool );
	}
}

GlobalDataPool::~GlobalDataPool() {
	MutexFactory::instance()->deleteMutex(mutex);
	for(GlobPoolMapIter it = this->globDataPool.begin();
	    it != this->globDataPool.end(); ++it )
	{
		delete it->second;
	}
}

// The function checks PID, type and array length before returning a copy of
// the PoolEntry. In failure case, it returns a temp-Entry with size 0 and NULL-ptr.
template <typename T> PoolEntry<T>* GlobalDataPool::getData( uint32_t data_pool_id,
		uint8_t sizeOrPosition ) {
	GlobPoolMapIter it = this->globDataPool.find( data_pool_id );
	if ( it != this->globDataPool.end() ) {
		PoolEntry<T>* entry = dynamic_cast< PoolEntry<T>* >( it->second );
		if (entry != nullptr ) {
			if ( sizeOrPosition <= entry->length ) {
				return entry;
			}
		}
	}
	return nullptr;
}

PoolEntryIF* GlobalDataPool::getRawData( uint32_t data_pool_id ) {
	GlobPoolMapIter it = this->globDataPool.find( data_pool_id );
	if ( it != this->globDataPool.end() ) {
		return it->second;
	} else {
		return nullptr;
	}
}

ReturnValue_t GlobalDataPool::freeDataPoolLock() {
	ReturnValue_t status = mutex->unlockMutex();
	if ( status != RETURN_OK ) {
		sif::error << "DataPool::DataPool: unlock of mutex failed with"
				" error code: " << status << std::endl;
	}
	return status;
}

ReturnValue_t GlobalDataPool::lockDataPool() {
	ReturnValue_t status = mutex->lockMutex(MutexIF::BLOCKING);
	if ( status != RETURN_OK ) {
		sif::error << "DataPool::DataPool: lock of mutex failed "
				"with error code: " << status << std::endl;
	}
	return status;
}

void GlobalDataPool::print() {
	sif::debug << "DataPool contains: " << std::endl;
	std::map<uint32_t, PoolEntryIF*>::iterator	dataPoolIt;
	dataPoolIt = this->globDataPool.begin();
	while( dataPoolIt != this->globDataPool.end() ) {
		sif::debug << std::hex << dataPoolIt->first << std::dec << " |";
		dataPoolIt->second->print();
		dataPoolIt++;
	}
}

uint32_t GlobalDataPool::PIDToDataPoolId(uint32_t parameter_id) {
	return (parameter_id >> 8) & 0x00FFFFFF;
}

uint8_t GlobalDataPool::PIDToArrayIndex(uint32_t parameter_id) {
	return (parameter_id & 0x000000FF);
}

uint32_t GlobalDataPool::poolIdAndPositionToPid(uint32_t poolId, uint8_t index) {
	return (poolId << 8) + index;
}


//SHOULDDO: Do we need a mutex lock here... I don't think so,
//as we only check static const values of elements in a list that do not change.
//there is no guarantee in the standard, but it seems to me that the implementation is safe -UM
ReturnValue_t GlobalDataPool::getType(uint32_t parameter_id, Type* type) {
	GlobPoolMapIter it = this->globDataPool.find( PIDToDataPoolId(parameter_id));
	if ( it != this->globDataPool.end() ) {
		*type = it->second->getType();
		return RETURN_OK;
	} else {
		*type = Type::UNKNOWN_TYPE;
		return RETURN_FAILED;
	}
}

bool GlobalDataPool::exists(uint32_t parameterId) {
	uint32_t poolId = PIDToDataPoolId(parameterId);
	uint32_t index = PIDToArrayIndex(parameterId);
	GlobPoolMapIter it = this->globDataPool.find( poolId );
	if (it != globDataPool.end()) {
		if (it->second->getSize() >= index) {
			return true;
		}
	}
	return false;
}

template PoolEntry<uint8_t>* GlobalDataPool::getData<uint8_t>(
		uint32_t data_pool_id, uint8_t size );
template PoolEntry<uint16_t>* GlobalDataPool::getData<uint16_t>(
		uint32_t data_pool_id, uint8_t size );
template PoolEntry<uint32_t>* GlobalDataPool::getData<uint32_t>(
		uint32_t data_pool_id, uint8_t size );
template PoolEntry<uint64_t>* GlobalDataPool::getData<uint64_t>(
		uint32_t data_pool_id, uint8_t size);
template PoolEntry<int8_t>* GlobalDataPool::getData<int8_t>(
		uint32_t data_pool_id, uint8_t size );
template PoolEntry<int16_t>* GlobalDataPool::getData<int16_t>(
		uint32_t data_pool_id, uint8_t size );
template PoolEntry<int32_t>* GlobalDataPool::getData<int32_t>(
		uint32_t data_pool_id, uint8_t size );
template PoolEntry<float>* GlobalDataPool::getData<float>(
		uint32_t data_pool_id, uint8_t size );
template PoolEntry<double>* GlobalDataPool::getData<double>(
		uint32_t data_pool_id, uint8_t size);
