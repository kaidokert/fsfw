#include "TcPacketStoredBase.h"

#include "../../../objectmanager/ObjectManager.h"
#include "../../../serviceinterface/ServiceInterface.h"
#include "../../../objectmanager/frameworkObjects.h"

#include <cstring>

StorageManagerIF* TcPacketStoredBase::store = nullptr;

TcPacketStoredBase::TcPacketStoredBase() {
    this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
    this->checkAndSetStore();

}

TcPacketStoredBase::~TcPacketStoredBase() {
}

ReturnValue_t TcPacketStoredBase::getData(const uint8_t ** dataPtr,
        size_t* dataSize) {
    auto result = this->store->getData(storeAddress, dataPtr, dataSize);
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TcPacketStoredBase: Could not get data!" << std::endl;
#else
        sif::printWarning("TcPacketStoredBase: Could not get data!\n");
#endif
    }
    return result;
}



bool TcPacketStoredBase::checkAndSetStore() {
    if (this->store == nullptr) {
        this->store = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
        if (this->store == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "TcPacketStoredBase::TcPacketStoredBase: TC Store not found!"
                    << std::endl;
#endif
            return false;
        }
    }
    return true;
}

void TcPacketStoredBase::setStoreAddress(store_address_t setAddress) {
    this->storeAddress = setAddress;
    const uint8_t* tempData = nullptr;
    size_t tempSize;
    ReturnValue_t status = StorageManagerIF::RETURN_FAILED;
    if (this->checkAndSetStore()) {
        status = this->store->getData(this->storeAddress, &tempData, &tempSize);
    }
    TcPacketBase* tcPacketBase = this->getPacketBase();
    if(tcPacketBase == nullptr) {
        return;
    }
    if (status == StorageManagerIF::RETURN_OK) {
        tcPacketBase->setData(tempData);
    }
    else {
        tcPacketBase->setData(nullptr);
        this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
    }
}

store_address_t TcPacketStoredBase::getStoreAddress() {
    return this->storeAddress;
}
