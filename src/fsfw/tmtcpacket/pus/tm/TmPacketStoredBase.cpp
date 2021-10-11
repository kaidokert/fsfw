#include "fsfw/tmtcpacket/pus/tm/TmPacketStoredBase.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

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
        store = ObjectManager::instance()->get<StorageManagerIF>(objects::TM_STORE);
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
        internalErrorReporter = ObjectManager::instance()->get<InternalErrorReporterIF>(
                objects::INTERNAL_ERROR_REPORTER);
    }
    if (internalErrorReporter != nullptr) {
        internalErrorReporter->lostTm();
    }
}

void TmPacketStoredBase::handleStoreFailure(const char *const packetType, ReturnValue_t result,
        size_t sizeToReserve) {
    checkAndReportLostTm();
#if FSFW_VERBOSE_LEVEL >= 1
    switch(result) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    case(StorageManagerIF::DATA_STORAGE_FULL): {
        sif::warning << "TmPacketStoredPus" << packetType << ": " <<
                "Store full for packet with size" << sizeToReserve << std::endl;
        break;
    }
    case(StorageManagerIF::DATA_TOO_LARGE): {
        sif::warning << "TmPacketStoredPus" << packetType << ": Data with size " <<
                sizeToReserve << " too large" <<  std::endl;
        break;
    }
#else
    case(StorageManagerIF::DATA_STORAGE_FULL): {
        sif::printWarning("TmPacketStoredPus%s: Store full for packet with "
                "size %d\n", packetType, sizeToReserve);
        break;
    }
    case(StorageManagerIF::DATA_TOO_LARGE): {
        sif::printWarning("TmPacketStoredPus%s: Data with size "
                "%d too large\n", packetType, sizeToReserve);
        break;
    }
#endif
#endif
    }
}
