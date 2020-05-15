#include <framework/storagemanager/StorageAccessor.h>

ConstStorageAccessor::ConstStorageAccessor(store_address_t storeId):
		storeId(storeId) {}

ConstStorageAccessor::ConstStorageAccessor(store_address_t storeId,
		StorageManagerIF* store):
		storeId(storeId), store(store) {
	internalState = AccessState::ASSIGNED;
}

ConstStorageAccessor::~ConstStorageAccessor() {
	if(deleteData and store != nullptr) {
		sif::debug << "deleting store data" << std::endl;
		store->deleteData(storeId);
	}
}

ConstStorageAccessor::ConstStorageAccessor(ConstStorageAccessor&& other):
		constDataPointer(other.constDataPointer), storeId(other.storeId),
		size_(other.size_), store(other.store), deleteData(other.deleteData),
		internalState(other.internalState) {
	// This prevent premature deletion
	other.store = nullptr;
}

ConstStorageAccessor& ConstStorageAccessor::operator=(
		ConstStorageAccessor&& other) {
	constDataPointer = other.constDataPointer;
	storeId = other.storeId;
	store = other.store;
	size_ = other.size_;
	deleteData = other.deleteData;
	this->store = other.store;
	// This prevents premature deletion
	other.store = nullptr;
	return *this;
}

const uint8_t* ConstStorageAccessor::data() const {
	return constDataPointer;
}

size_t ConstStorageAccessor::size() const {
	if(internalState == AccessState::UNINIT) {
		sif::warning << "StorageAccessor: Not initialized!" << std::endl;
	}
	return size_;
}

ReturnValue_t ConstStorageAccessor::getDataCopy(uint8_t *pointer,
		size_t maxSize) {
	if(internalState == AccessState::UNINIT) {
		sif::warning << "StorageAccessor: Not initialized!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	if(size_ > maxSize) {
		sif::error << "StorageAccessor: Supplied buffer not large enough" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	std::copy(constDataPointer, constDataPointer + size_, pointer);
	return HasReturnvaluesIF::RETURN_OK;
}

void ConstStorageAccessor::release() {
	deleteData = false;
}

store_address_t ConstStorageAccessor::getId() const {
	return storeId;
}

void ConstStorageAccessor::print() const {
	if(internalState == AccessState::UNINIT) {
		sif::warning << "StorageAccessor: Not initialized!" << std::endl;
		return;
	}
	sif::info << "StorageAccessor: Printing data: [";
	for(uint16_t iPool = 0; iPool < size_; iPool++) {
		sif::info << std::hex << (int)constDataPointer[iPool];
		if(iPool < size_ - 1){
			sif::info << " , ";
		}
	}
	sif::info << " ] " << std::endl;
}

void ConstStorageAccessor::assignStore(StorageManagerIF* store) {
	internalState = AccessState::ASSIGNED;
	this->store = store;
}


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
		sif::error << "StorageAccessor: Supplied buffer not large enough" << std::endl;
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
		sif::error << "StorageAccessor: Data too large for pool entry!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	std::copy(data, data + size, dataPointer + offset);
	return HasReturnvaluesIF::RETURN_OK;
}

void StorageAccessor::assignConstPointer() {
	constDataPointer = dataPointer;
}


