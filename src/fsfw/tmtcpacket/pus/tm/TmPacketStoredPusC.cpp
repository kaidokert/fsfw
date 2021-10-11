#include "fsfw/tmtcpacket/pus/tm/TmPacketStoredPusC.h"

#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

#include <cstring>

TmPacketStoredPusC::TmPacketStoredPusC(store_address_t setAddress) :
        TmPacketStoredBase(setAddress), TmPacketPusC(nullptr){
}

TmPacketStoredPusC::TmPacketStoredPusC(uint16_t apid, uint8_t service,
        uint8_t subservice, uint16_t packetSubcounter, const uint8_t *data,
        uint32_t size, const uint8_t *headerData, uint32_t headerSize, uint16_t destinationId,
        uint8_t timeRefField) :
        TmPacketPusC(nullptr) {
    storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
    if (not TmPacketStoredBase::checkAndSetStore()) {
        return;
    }
    uint8_t *pData = nullptr;
    ReturnValue_t returnValue = store->getFreeElement(&storeAddress,
            (getPacketMinimumSize() + size + headerSize), &pData);

    if (returnValue != store->RETURN_OK) {
        TmPacketStoredBase::checkAndReportLostTm();
        return;
    }
    setData(pData);
    initializeTmPacket(apid, service, subservice, packetSubcounter, destinationId, timeRefField);
    memcpy(getSourceData(), headerData, headerSize);
    memcpy(getSourceData() + headerSize, data, size);
    setPacketDataLength(size + headerSize + sizeof(PUSTmDataFieldHeaderPusC) + CRC_SIZE - 1);
}

TmPacketStoredPusC::TmPacketStoredPusC(uint16_t apid, uint8_t service,
        uint8_t subservice, uint16_t packetSubcounter, SerializeIF *content,
        SerializeIF *header, uint16_t destinationId, uint8_t timeRefField) :
        TmPacketPusC(nullptr) {
    storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
    if (not TmPacketStoredBase::checkAndSetStore()) {
        return;
    }
    size_t sourceDataSize = 0;
    if (content != NULL) {
        sourceDataSize += content->getSerializedSize();
    }
    if (header != NULL) {
        sourceDataSize += header->getSerializedSize();
    }
    uint8_t *p_data = NULL;
    ReturnValue_t returnValue = store->getFreeElement(&storeAddress,
            (getPacketMinimumSize() + sourceDataSize), &p_data);
    if (returnValue != store->RETURN_OK) {
        TmPacketStoredBase::checkAndReportLostTm();
    }
    setData(p_data);
    initializeTmPacket(apid, service, subservice, packetSubcounter, destinationId, timeRefField);
    uint8_t *putDataHere = getSourceData();
    size_t size = 0;
    if (header != NULL) {
        header->serialize(&putDataHere, &size, sourceDataSize,
                SerializeIF::Endianness::BIG);
    }
    if (content != NULL) {
        content->serialize(&putDataHere, &size, sourceDataSize,
                SerializeIF::Endianness::BIG);
    }
    setPacketDataLength(
            sourceDataSize + sizeof(PUSTmDataFieldHeaderPusC) + CRC_SIZE - 1);
}

uint8_t* TmPacketStoredPusC::getAllTmData() {
    return getWholeData();
}

void TmPacketStoredPusC::setDataPointer(const uint8_t *newPointer) {
    setData(newPointer);
}
