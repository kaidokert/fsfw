#include "TcPacketPus.h"
#include "../../../globalfunctions/CRC.h"

#include <cstring>

TcPacketPus::TcPacketPus(const uint8_t *setData): TcPacketBase(setData) {
    tcData = reinterpret_cast<TcPacketPointer*>(const_cast<uint8_t*>(setData));
}

uint8_t TcPacketPus::getService() const {
    return tcData->dataField.serviceType;
}

uint8_t TcPacketPus::getSubService() {
    return tcData->dataField.serviceSubtype;
}

uint8_t TcPacketPus::getAcknowledgeFlags() {
    return tcData->dataField.versionTypeAck & 0b00001111;
}

const uint8_t* TcPacketPus::getApplicationData() const {
    return &tcData->appData;
}

uint16_t TcPacketPus::getApplicationDataSize() {
    return getPacketDataLength() - sizeof(tcData->dataField) - CRC_SIZE + 1;
}

uint16_t TcPacketPus::getErrorControl() {
    uint16_t size = getApplicationDataSize() + CRC_SIZE;
    uint8_t* p_to_buffer = &tcData->appData;
    return (p_to_buffer[size - 2] << 8) + p_to_buffer[size - 1];
}

void TcPacketPus::setErrorControl() {
    uint32_t full_size = getFullSize();
    uint16_t crc = CRC::crc16ccitt(getWholeData(), full_size - CRC_SIZE);
    uint32_t size = getApplicationDataSize();
    (&tcData->appData)[size] = (crc & 0XFF00) >> 8; // CRCH
    (&tcData->appData)[size + 1] = (crc) & 0X00FF;      // CRCL
}

void TcPacketPus::setData(const uint8_t* pData) {
    SpacePacketBase::setData(pData);
    // This function is const-correct, but it was decided to keep the pointer non-const
    // for convenience. Therefore, cast aways constness here and then cast to packet type.
    tcData = reinterpret_cast<TcPacketPointer*>(const_cast<uint8_t*>(pData));
}

uint8_t TcPacketPus::getSecondaryHeaderFlag() {
    return (tcData->dataField.versionTypeAck & 0b10000000) >> 7;
}

uint8_t TcPacketPus::getPusVersionNumber() {
    return (tcData->dataField.versionTypeAck & 0b01110000) >> 4;
}

void TcPacketPus::initializeTcPacket(uint16_t apid, uint16_t sequenceCount,
        uint8_t ack, uint8_t service, uint8_t subservice) {
    initSpacePacketHeader(true, true, apid, sequenceCount);
    std::memset(&tcData->dataField, 0, sizeof(tcData->dataField));
    setPacketDataLength(sizeof(PUSTcDataFieldHeader) + CRC_SIZE - 1);
    // Data Field Header:
    // Set CCSDS_secondary_header_flag to 0 and version number to 001
    tcData->dataField.versionTypeAck = 0b00010000;
    tcData->dataField.versionTypeAck |= (ack & 0x0F);
    tcData->dataField.serviceType = service;
    tcData->dataField.serviceSubtype = subservice;
}

size_t TcPacketPus::calculateFullPacketLength(size_t appDataLen) {
    return sizeof(CCSDSPrimaryHeader) + sizeof(PUSTcDataFieldHeader) +
            appDataLen + TcPacketBase::CRC_SIZE;
}
