#include "TcPacketStoredPus.h"

#include "../../../serviceinterface/ServiceInterface.h"

#include <cstring>

TcPacketStoredPus::TcPacketStoredPus(uint16_t apid,  uint8_t service,
        uint8_t subservice, uint8_t sequenceCount, const uint8_t* data,
        size_t size, uint8_t ack) :
        TcPacketPus(nullptr) {
    this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
    if (not this->checkAndSetStore()) {
        return;
    }
    uint8_t* pData = nullptr;
    ReturnValue_t returnValue = this->store->getFreeElement(&this->storeAddress,
            (TC_PACKET_MIN_SIZE + size), &pData);
    if (returnValue != this->store->RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TcPacketStoredBase: Could not get free element from store!"
                << std::endl;
#endif
        return;
    }
    this->setData(pData);
    initializeTcPacket(apid, sequenceCount, ack, service, subservice);
    std::memcpy(&tcData->appData, data, size);
    this->setPacketDataLength(
            size + sizeof(PUSTcDataFieldHeader) + CRC_SIZE - 1);
    this->setErrorControl();
}

TcPacketStoredPus::TcPacketStoredPus(): TcPacketStoredBase(), TcPacketPus(nullptr) {
}

TcPacketStoredPus::TcPacketStoredPus(store_address_t setAddress): TcPacketPus(nullptr) {
    TcPacketStoredBase::setStoreAddress(setAddress);
}

TcPacketStoredPus::TcPacketStoredPus(const uint8_t* data, size_t size): TcPacketPus(data) {
    if (this->getFullSize() != size) {
        return;
    }
    if (this->checkAndSetStore()) {
        ReturnValue_t status = store->addData(&storeAddress, data, size);
        if (status != HasReturnvaluesIF::RETURN_OK) {
            this->setData(nullptr);
        }
        const uint8_t* storePtr = nullptr;
        // Repoint base data pointer to the data in the store.
        store->getData(storeAddress, &storePtr, &size);
        this->setData(storePtr);
    }
}

ReturnValue_t TcPacketStoredPus::deletePacket() {
    ReturnValue_t result = this->store->deleteData(this->storeAddress);
    this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
    this->setData(nullptr);
    return result;
}

TcPacketBase* TcPacketStoredPus::getPacketBase() {
    return this;
}


bool TcPacketStoredPus::isSizeCorrect() {
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
