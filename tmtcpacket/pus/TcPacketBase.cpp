#include "TcPacketBase.h"

#include "../../globalfunctions/CRC.h"
#include "../../globalfunctions/arrayprinter.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

#include <cstring>

TcPacketBase::TcPacketBase(const uint8_t* setData) :
		SpacePacketBase(setData) {
	tcData = reinterpret_cast<TcPacketPointer*>(const_cast<uint8_t*>(setData));
}

TcPacketBase::~TcPacketBase() {
	//Nothing to do.
}

uint8_t TcPacketBase::getService() {
	return tcData->dataField.service_type;
}

uint8_t TcPacketBase::getSubService() {
	return tcData->dataField.service_subtype;
}

uint8_t TcPacketBase::getAcknowledgeFlags() {
	return tcData->dataField.version_type_ack & 0b00001111;
}

const uint8_t* TcPacketBase::getApplicationData() const {
	return &tcData->appData;
}

uint16_t TcPacketBase::getApplicationDataSize() {
	return getPacketDataLength() - sizeof(tcData->dataField) - CRC_SIZE + 1;
}

uint16_t TcPacketBase::getErrorControl() {
	uint16_t size = getApplicationDataSize() + CRC_SIZE;
	uint8_t* p_to_buffer = &tcData->appData;
	return (p_to_buffer[size - 2] << 8) + p_to_buffer[size - 1];
}

void TcPacketBase::setErrorControl() {
	uint32_t full_size = getFullSize();
	uint16_t crc = CRC::crc16ccitt(getWholeData(), full_size - CRC_SIZE);
	uint32_t size = getApplicationDataSize();
	(&tcData->appData)[size] = (crc & 0XFF00) >> 8;	// CRCH
	(&tcData->appData)[size + 1] = (crc) & 0X00FF; 		// CRCL
}

void TcPacketBase::setData(const uint8_t* pData) {
	SpacePacketBase::setData(pData);
	tcData = (TcPacketPointer*) pData;
}

uint8_t TcPacketBase::getSecondaryHeaderFlag() {
	return (tcData->dataField.version_type_ack & 0b10000000) >> 7;
}

uint8_t TcPacketBase::getPusVersionNumber() {
	return (tcData->dataField.version_type_ack & 0b01110000) >> 4;
}

void TcPacketBase::print() {
    sif::debug << "TcPacketBase::print: " << std::endl;
	arrayprinter::print(getWholeData(), getFullSize());
}

void TcPacketBase::initializeTcPacket(uint16_t apid, uint16_t sequenceCount,
        uint8_t ack, uint8_t service, uint8_t subservice) {
	initSpacePacketHeader(true, true, apid, sequenceCount);
	std::memset(&tcData->dataField, 0, sizeof(tcData->dataField));
	setPacketDataLength(sizeof(PUSTcDataFieldHeader) + CRC_SIZE - 1);
	//Data Field Header:
	//Set CCSDS_secondary_header_flag to 0 and version number to 001
	tcData->dataField.version_type_ack = 0b00010000;
	tcData->dataField.version_type_ack |= (ack & 0x0F);
	tcData->dataField.service_type = service;
	tcData->dataField.service_subtype = subservice;
}

size_t TcPacketBase::calculateFullPacketLength(size_t appDataLen) {
	return sizeof(CCSDSPrimaryHeader) + sizeof(PUSTcDataFieldHeader) +
			appDataLen + TcPacketBase::CRC_SIZE;
}
