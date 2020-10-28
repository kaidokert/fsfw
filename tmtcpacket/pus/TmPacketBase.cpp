#include "TmPacketBase.h"

#include "../../globalfunctions/CRC.h"
#include "../../globalfunctions/arrayprinter.h"
#include "../../objectmanager/ObjectManagerIF.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../timemanager/CCSDSTime.h"

#include <cstring>

TimeStamperIF* TmPacketBase::timeStamper = nullptr;
object_id_t TmPacketBase::timeStamperId = 0;

TmPacketBase::TmPacketBase(uint8_t* setData) :
		SpacePacketBase(setData) {
	tmData = reinterpret_cast<TmPacketPointer*>(setData);
}

TmPacketBase::~TmPacketBase() {
	//Nothing to do.
}

uint8_t TmPacketBase::getService() {
	return tmData->data_field.service_type;
}

uint8_t TmPacketBase::getSubService() {
	return tmData->data_field.service_subtype;
}

uint8_t* TmPacketBase::getSourceData() {
	return &tmData->data;
}

uint16_t TmPacketBase::getSourceDataSize() {
	return getPacketDataLength() - sizeof(tmData->data_field)
			- CRC_SIZE + 1;
}

uint16_t TmPacketBase::getErrorControl() {
	uint32_t size = getSourceDataSize() + CRC_SIZE;
	uint8_t* p_to_buffer = &tmData->data;
	return (p_to_buffer[size - 2] << 8) + p_to_buffer[size - 1];
}

void TmPacketBase::setErrorControl() {
	uint32_t full_size = getFullSize();
	uint16_t crc = CRC::crc16ccitt(getWholeData(), full_size - CRC_SIZE);
	uint32_t size = getSourceDataSize();
	getSourceData()[size] = (crc & 0XFF00) >> 8;	// CRCH
	getSourceData()[size + 1] = (crc) & 0X00FF; 		// CRCL
}

void TmPacketBase::setData(const uint8_t* p_Data) {
	SpacePacketBase::setData(p_Data);
	tmData = (TmPacketPointer*) p_Data;
}

void TmPacketBase::print() {
	 sif::debug << "TmPacketBase::print: " << std::endl;
	 arrayprinter::print(getWholeData(), getFullSize());
}

bool TmPacketBase::checkAndSetStamper() {
	if (timeStamper == NULL) {
		timeStamper = objectManager->get<TimeStamperIF>(timeStamperId);
		if (timeStamper == NULL) {
			sif::error << "TmPacketBase::checkAndSetStamper: Stamper not found!"
					<< std::endl;
			return false;
		}
	}
	return true;
}

ReturnValue_t TmPacketBase::getPacketTime(timeval* timestamp) const {
	uint32_t tempSize = 0;
	return CCSDSTime::convertFromCcsds(timestamp, tmData->data_field.time,
			&tempSize, sizeof(tmData->data_field.time));
}

uint8_t* TmPacketBase::getPacketTimeRaw() const{
	return tmData->data_field.time;

}

void TmPacketBase::initializeTmPacket(uint16_t apid, uint8_t service,
        uint8_t subservice, uint8_t packetSubcounter) {
	//Set primary header:
	initSpacePacketHeader(false, true, apid);
	//Set data Field Header:
	//First, set to zero.
	memset(&tmData->data_field, 0, sizeof(tmData->data_field));

	// NOTE: In PUS-C, the PUS Version is 2 and specified for the first 4 bits.
	// The other 4 bits of the first byte are the spacecraft time reference
	// status. To change to PUS-C, set 0b00100000.
    // Set CCSDS_secondary header flag to 0, version number to 001 and ack
    // to 0000
	tmData->data_field.version_type_ack = 0b00010000;
	tmData->data_field.service_type = service;
	tmData->data_field.service_subtype = subservice;
	tmData->data_field.subcounter = packetSubcounter;
	//Timestamp packet
	if (checkAndSetStamper()) {
		timeStamper->addTimeStamp(tmData->data_field.time,
		        sizeof(tmData->data_field.time));
	}
}

void TmPacketBase::setSourceDataSize(uint16_t size) {
	setPacketDataLength(size + sizeof(PUSTmDataFieldHeader) + CRC_SIZE - 1);
}

size_t TmPacketBase::getTimestampSize() const {
	return sizeof(tmData->data_field.time);
}
