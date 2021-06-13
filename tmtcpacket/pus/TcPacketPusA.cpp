#include "TcPacketPusA.h"
#include "../../globalfunctions/CRC.h"

#include <cstring>

TcPacketPusA::TcPacketPusA(const uint8_t *setData): TcPacketBase(setData) {
    tcData = reinterpret_cast<TcPacketPointer*>(const_cast<uint8_t*>(setData));
}

uint8_t TcPacketPusA::getService() const {
    return tcData->dataField.service_type;
}

uint8_t TcPacketPusA::getSubService() {
    return tcData->dataField.service_subtype;
}

uint8_t TcPacketPusA::getAcknowledgeFlags() {
    return tcData->dataField.version_type_ack & 0b00001111;
}

const uint8_t* TcPacketPusA::getApplicationData() const {
    return &tcData->appData;
}

uint16_t TcPacketPusA::getApplicationDataSize() {
    return getPacketDataLength() - sizeof(tcData->dataField) - CRC_SIZE + 1;
}

uint16_t TcPacketPusA::getErrorControl() {
    uint16_t size = getApplicationDataSize() + CRC_SIZE;
    uint8_t* p_to_buffer = &tcData->appData;
    return (p_to_buffer[size - 2] << 8) + p_to_buffer[size - 1];
}

void TcPacketPusA::setErrorControl() {
    uint32_t full_size = getFullSize();
    uint16_t crc = CRC::crc16ccitt(getWholeData(), full_size - CRC_SIZE);
    uint32_t size = getApplicationDataSize();
    (&tcData->appData)[size] = (crc & 0XFF00) >> 8; // CRCH
    (&tcData->appData)[size + 1] = (crc) & 0X00FF;      // CRCL
}

void TcPacketPusA::setData(const uint8_t* pData) {
    SpacePacketBase::setData(pData);
    // This function is const-correct, but it was decided to keep the pointer non-const
    // for convenience. Therefore, cast aways constness here and then cast to packet type.
    tcData = reinterpret_cast<TcPacketPointer*>(const_cast<uint8_t*>(pData));
}

uint8_t TcPacketPusA::getSecondaryHeaderFlag() {
    return (tcData->dataField.version_type_ack & 0b10000000) >> 7;
}

uint8_t TcPacketPusA::getPusVersionNumber() {
    return (tcData->dataField.version_type_ack & 0b01110000) >> 4;
}

void TcPacketPusA::initializeTcPacket(uint16_t apid, uint16_t sequenceCount,
        uint8_t ack, uint8_t service, uint8_t subservice) {
    initSpacePacketHeader(true, true, apid, sequenceCount);
    std::memset(&tcData->dataField, 0, sizeof(tcData->dataField));
    setPacketDataLength(sizeof(PUSTcDataFieldHeader) + CRC_SIZE - 1);
    // Data Field Header:
    // Set CCSDS_secondary_header_flag to 0 and version number to 001
    tcData->dataField.version_type_ack = 0b00010000;
    tcData->dataField.version_type_ack |= (ack & 0x0F);
    tcData->dataField.service_type = service;
    tcData->dataField.service_subtype = subservice;
}

size_t TcPacketPusA::calculateFullPacketLength(size_t appDataLen) {
    return sizeof(CCSDSPrimaryHeader) + sizeof(PUSTcDataFieldHeader) +
            appDataLen + TcPacketBase::CRC_SIZE;
}
