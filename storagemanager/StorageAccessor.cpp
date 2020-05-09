#include <test/prototypes/StorageAccessor.h>

ConstStorageAccessor::ConstStorageAccessor(store_address_t storeId): storeId(storeId) {}

ConstStorageAccessor::~ConstStorageAccessor() {
	if(deleteData and store != nullptr) {
		sif::debug << "deleting store data" << std::endl;
		store->deleteDataNonLocking(storeId);
	}
	if(mutexLock != nullptr) {
		sif::debug << "unlocking mutex lock" << std::endl;
		mutexLock.reset();
	}
}

ConstStorageAccessor& ConstStorageAccessor::operator=(
		ConstStorageAccessor&& other) {
	constDataPointer = other.constDataPointer;
	storeId = other.storeId;
	store = other.store;
	size_ = other.size_;
	deleteData = other.deleteData;
	this->store = other.store;
	// Transfer ownership of the lock.
	mutexLock = std::move(other.mutexLock);
	// This prevents double deletion of the resource
	other.mutexLock = nullptr;
	// This prevent premature deletion
	other.store = nullptr;
	return *this;
}

StorageAccessor::StorageAccessor(store_address_t storeId):
		ConstStorageAccessor(storeId) {
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

ConstStorageAccessor::ConstStorageAccessor(ConstStorageAccessor&& other):
		constDataPointer(other.constDataPointer), storeId(other.storeId),
		size_(other.size_), store(other.store), deleteData(other.deleteData),
		internalState(other.internalState) {
	// Transfer ownership of the lock.
	mutexLock = std::move(other.mutexLock);
	// This prevents double deletion of the resource. Not strictly necessary,
	// from the testing I have conducted so far but I am not familiar enough
	// with move semantics so I will just set the other lock to nullptr for now.
	other.mutexLock = nullptr;
	// This prevent premature deletion
	other.store = nullptr;
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

void ConstStorageAccessor::getDataCopy(uint8_t *pointer) {
	if(internalState == AccessState::UNINIT) {
		sif::warning << "StorageAccessor: Not initialized!" << std::endl;
		return;
	}
	std::copy(constDataPointer, constDataPointer + size_, pointer);
}

void ConstStorageAccessor::release() {
	deleteData = false;
}

ReturnValue_t ConstStorageAccessor::lock(MutexIF* mutex, uint32_t mutexTimeout) {
	if(mutexLock == nullptr) {
		mutexLock = std::unique_ptr<MutexHelper>(new MutexHelper(mutex, mutexTimeout));
		return mutexLock.get()->getResult();
	}
	else {
		sif::warning << "StorageAccessor: Attempted to lock twice. Check code!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

void ConstStorageAccessor::unlock() {
	if(mutexLock != nullptr) {
		mutexLock.reset();
	}
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
	internalState = AccessState::READ;
	this->store = store;
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
	std::copy(data, data + size, dataPointer);
	return HasReturnvaluesIF::RETURN_OK;
}

void StorageAccessor::assignConstPointer() {
	constDataPointer = dataPointer;
}


