#include "StorageAccessor.h"
#include "StorageManagerIF.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

StorageAccessor::StorageAccessor(store_address_t storeId):
		ConstStorageAccessor(storeId) {
}

StorageAccessor::StorageAccessor(store_address_t storeId,
		StorageManagerIF* store):
		ConstStorageAccessor(storeId, store) {
}

StorageAccessor& StorageAccessor::operator =(
		StorageAccessor&& other) {
	// Call the parent move assignment and also assign own member.
	dataPointer = other.dataPointer;
	StorageAccessor::operator=(std::move(other));
	return * this;
}

// Call the parent move ctor and also transfer own member.
StorageAccessor::StorageAccessor(StorageAccessor&& other):
		ConstStorageAccessor(std::move(other)), dataPointer(other.dataPointer) {
}

ReturnValue_t StorageAccessor::getDataCopy(uint8_t *pointer, size_t maxSize) {
	if(internalState == AccessState::UNINIT) {
		sif::warning << "StorageAccessor: Not initialized!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	if(size_ > maxSize) {
		sif::error << "StorageAccessor: Supplied buffer not large "
				"enough" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	std::copy(dataPointer, dataPointer + size_, pointer);
	return HasReturnvaluesIF::RETURN_OK;
}

uint8_t* StorageAccessor::data() {
	if(internalState == AccessState::UNINIT) {
		sif::warning << "StorageAccessor: Not initialized!" << std::endl;
	}
	return dataPointer;
}

ReturnValue_t StorageAccessor::write(uint8_t *data, size_t size,
		uint16_t offset) {
	if(internalState == AccessState::UNINIT) {
		sif::warning << "StorageAccessor: Not initialized!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	if(offset + size > size_) {
		sif::error << "StorageAccessor: Data too large for pool "
				"entry!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	std::copy(data, data + size, dataPointer + offset);
	return HasReturnvaluesIF::RETURN_OK;
}

void StorageAccessor::assignConstPointer() {
	constDataPointer = dataPointer;
}


