#include "TmPacketStoredBase.h"

#include "../../objectmanager/ObjectManagerIF.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../tmtcservices/TmTcMessage.h"

#include <cstring>

StorageManagerIF *TmPacketStoredBase::store = nullptr;
InternalErrorReporterIF *TmPacketStoredBase::internalErrorReporter = nullptr;

TmPacketStoredBase::TmPacketStoredBase(store_address_t setAddress): storeAddress(setAddress) {
    setStoreAddress(storeAddress);
}

TmPacketStoredBase::TmPacketStoredBase() {
}


TmPacketStoredBase::~TmPacketStoredBase() {
}

store_address_t TmPacketStoredBase::getStoreAddress() {
    return storeAddress;
}

void TmPacketStoredBase::deletePacket() {
    store->deleteData(storeAddress);
    storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
    setDataPointer(nullptr);
}

void TmPacketStoredBase::setStoreAddress(store_address_t setAddress) {
    storeAddress = setAddress;
    const uint8_t* tempData = nullptr;
    size_t tempSize;
    if (not checkAndSetStore()) {
        return;
    }
    ReturnValue_t status = store->getData(storeAddress, &tempData, &tempSize);
    if (status == StorageManagerIF::RETURN_OK) {
        setDataPointer(tempData);
    } else {
        setDataPointer(nullptr);
        storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
    }
}

bool TmPacketStoredBase::checkAndSetStore() {
    if (store == nullptr) {
        store = objectManager->get<StorageManagerIF>(objects::TM_STORE);
        if (store == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "TmPacketStored::TmPacketStored: TM Store not found!"
                    << std::endl;
#endif
            return false;
        }
    }
    return true;
}

ReturnValue_t TmPacketStoredBase::sendPacket(MessageQueueId_t destination,
        MessageQueueId_t sentFrom, bool doErrorReporting) {
    if (getAllTmData() == nullptr) {
        //SHOULDDO: More decent code.
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    TmTcMessage tmMessage(getStoreAddress());
    ReturnValue_t result = MessageQueueSenderIF::sendMessage(destination,
            &tmMessage, sentFrom);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        deletePacket();
        if (doErrorReporting) {
            checkAndReportLostTm();
        }
        return result;
    }
    //SHOULDDO: In many cases, some counter is incremented for successfully sent packets. The check is often not done, but just incremented.
    return HasReturnvaluesIF::RETURN_OK;

}

void TmPacketStoredBase::checkAndReportLostTm() {
    if (internalErrorReporter == nullptr) {
        internalErrorReporter = objectManager->get<InternalErrorReporterIF>(
                objects::INTERNAL_ERROR_REPORTER);
    }
    if (internalErrorReporter != nullptr) {
        internalErrorReporter->lostTm();
    }
}


