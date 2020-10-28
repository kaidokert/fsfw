#include "TcPacketStored.h"
#include "../../objectmanager/ObjectManagerIF.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

#include <cstring>

StorageManagerIF* TcPacketStored::store = nullptr;

TcPacketStored::TcPacketStored(store_address_t setAddress) :
		TcPacketBase(nullptr), storeAddress(setAddress) {
	setStoreAddress(storeAddress);
}

TcPacketStored::TcPacketStored(uint16_t apid,  uint8_t service,
        uint8_t subservice, uint8_t sequenceCount, const uint8_t* data,
		size_t size, uint8_t ack) :
		TcPacketBase(nullptr) {
	this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	if (not this->checkAndSetStore()) {
		return;
	}
	uint8_t* pData = nullptr;
	ReturnValue_t returnValue = this->store->getFreeElement(&this->storeAddress,
			(TC_PACKET_MIN_SIZE + size), &pData);
	if (returnValue != this->store->RETURN_OK) {
		sif::warning << "TcPacketStored: Could not get free element from store!"
				<< std::endl;
		return;
	}
	this->setData(pData);
	initializeTcPacket(apid, sequenceCount, ack, service, subservice);
	memcpy(&tcData->appData, data, size);
	this->setPacketDataLength(
			size + sizeof(PUSTcDataFieldHeader) + CRC_SIZE - 1);
	this->setErrorControl();
}

ReturnValue_t TcPacketStored::getData(const uint8_t ** dataPtr,
		size_t* dataSize) {
	auto result = this->store->getData(storeAddress, dataPtr, dataSize);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		sif::warning << "TcPacketStored: Could not get data!" << std::endl;
	}
	return result;
}

TcPacketStored::TcPacketStored(): TcPacketBase(nullptr) {
	this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	this->checkAndSetStore();

}

ReturnValue_t TcPacketStored::deletePacket() {
	ReturnValue_t result = this->store->deleteData(this->storeAddress);
	this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	this->setData(nullptr);
	return result;
}

bool TcPacketStored::checkAndSetStore() {
	if (this->store == nullptr) {
		this->store = objectManager->get<StorageManagerIF>(objects::TC_STORE);
		if (this->store == nullptr) {
			sif::error << "TcPacketStored::TcPacketStored: TC Store not found!"
					<< std::endl;
			return false;
		}
	}
	return true;
}

void TcPacketStored::setStoreAddress(store_address_t setAddress) {
	this->storeAddress = setAddress;
	const uint8_t* tempData = nullptr;
	size_t temp_size;
	ReturnValue_t status = StorageManagerIF::RETURN_FAILED;
	if (this->checkAndSetStore()) {
		status = this->store->getData(this->storeAddress, &tempData,
				&temp_size);
	}
	if (status == StorageManagerIF::RETURN_OK) {
		this->setData(tempData);
	} else {
		this->setData(nullptr);
		this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	}
}

store_address_t TcPacketStored::getStoreAddress() {
	return this->storeAddress;
}

bool TcPacketStored::isSizeCorrect() {
	const uint8_t* temp_data = nullptr;
	size_t temp_size;
	ReturnValue_t status = this->store->getData(this->storeAddress, &temp_data,
			&temp_size);
	if (status == StorageManagerIF::RETURN_OK) {
		if (this->getFullSize() == temp_size) {
			return true;
		}
	}
	return false;
}

TcPacketStored::TcPacketStored(const uint8_t* data, uint32_t size) :
		TcPacketBase(data) {
	if (getFullSize() != size) {
		return;
	}
	if (this->checkAndSetStore()) {
		ReturnValue_t status = store->addData(&storeAddress, data, size);
		if (status != HasReturnvaluesIF::RETURN_OK) {
			this->setData(nullptr);
		}
	}
}
