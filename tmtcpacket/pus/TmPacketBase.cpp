#include <framework/globalfunctions/CRC.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tmtcpacket/pus/TmPacketBase.h>
#include <framework/timemanager/CCSDSTime.h>
#include <string.h>

TmPacketBase::TmPacketBase(uint8_t* set_data) :
		SpacePacketBase(set_data) {
	tm_data = (TmPacketPointer*) set_data;
}

TmPacketBase::~TmPacketBase() {
	//Nothing to do.
}

uint8_t TmPacketBase::getService() {
	return tm_data->data_field.service_type;
}

uint8_t TmPacketBase::getSubService() {
	return tm_data->data_field.service_subtype;
}

uint8_t* TmPacketBase::getSourceData() {
	return &tm_data->data;
}

uint16_t TmPacketBase::getSourceDataSize() {
	return getPacketDataLength() - sizeof(tm_data->data_field)
			- CRC_SIZE + 1;
}

uint16_t TmPacketBase::getErrorControl() {
	uint32_t size = getSourceDataSize() + CRC_SIZE;
	uint8_t* p_to_buffer = &tm_data->data;
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
	tm_data = (TmPacketPointer*) p_Data;
}

void TmPacketBase::print() {
	/*uint8_t * wholeData = getWholeData();
	 debug << "TmPacket contains: " << std::endl;
	 for (uint8_t count = 0; count < getFullSize(); ++count ) {
	 debug << std::hex << (uint16_t)wholeData[count] << " ";
	 }
	 debug << std::dec << std::endl;*/
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
	return CCSDSTime::convertFromCcsds(timestamp, tm_data->data_field.time,
			&tempSize, sizeof(tm_data->data_field.time));
}

uint8_t* TmPacketBase::getPacketTimeRaw() const{
	return tm_data->data_field.time;

}

void TmPacketBase::initializeTmPacket(uint16_t apid, uint8_t service, uint8_t subservice, uint8_t packetSubcounter) {
	//Set primary header:
	initSpacePacketHeader(false, true, apid);
	//Set data Field Header:
	//First, set to zero.
	memset(&tm_data->data_field, 0, sizeof(tm_data->data_field));
	//Set CCSDS_secondary header flag to 0, version number to 001 and ack to 0000
	// NOTE: In PUS-C, the PUS Version is 2 and specified for the first 4 bits.
	// The other 4 bits of the first byte are the spacecraft time reference status
	// To change to PUS-C, set 0b00100000
	tm_data->data_field.version_type_ack = 0b00010000;
	tm_data->data_field.service_type = service;
	tm_data->data_field.service_subtype = subservice;
	tm_data->data_field.subcounter = packetSubcounter;
	//Timestamp packet
	if (checkAndSetStamper()) {
		timeStamper->addTimeStamp(tm_data->data_field.time, sizeof(tm_data->data_field.time));
	}
}

void TmPacketBase::setSourceData(uint8_t* sourceData, size_t sourceSize) {
	memcpy(getSourceData(), sourceData, sourceSize);
	setSourceDataSize(sourceSize);
}

void TmPacketBase::setSourceDataSize(uint16_t size) {
	setPacketDataLength(size + sizeof(PUSTmDataFieldHeader) + CRC_SIZE - 1);
}

uint32_t TmPacketBase::getTimestampSize() const {
	return sizeof(tm_data->data_field.time);
}

TimeStamperIF* TmPacketBase::timeStamper = NULL;
object_id_t TmPacketBase::timeStamperId = 0;
