#include <framework/storagemanager/ConstStoreAccessor.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/storagemanager/StorageManagerIF.h>

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
