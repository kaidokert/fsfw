/*
 * TmPacketBase.cpp
 *
 *  Created on: 18.06.2012
 *      Author: baetz
 */

#include <framework/globalfunctions/crc_ccitt.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tmtcpacket/pus/TmPacketBase.h>

TmPacketBase::TmPacketBase( uint8_t* set_data ) : SpacePacketBase( set_data ) {
	this->tm_data = (TmPacketPointer*)set_data;
}

TmPacketBase::~TmPacketBase() {
	//Nothing to do.
}

uint8_t TmPacketBase::getService() {
	return this->tm_data->data_field.service_type;
}

uint8_t TmPacketBase::getSubService() {
	return this->tm_data->data_field.service_subtype;
}

uint8_t* TmPacketBase::getSourceData() {
	return &this->tm_data->data;
}

uint16_t TmPacketBase::getSourceDataSize() {
	return this->getPacketDataLength() - sizeof(this->tm_data->data_field) - CRC_SIZE  + 1;
}

uint16_t TmPacketBase::getErrorControl() {
	uint32_t size = this->getSourceDataSize() + CRC_SIZE;
	uint8_t* p_to_buffer = &this->tm_data->data;
	return ( p_to_buffer[size - 2] << 8 ) + p_to_buffer[size - 1];
}


void TmPacketBase::setErrorControl() {
	uint32_t full_size = this->getFullSize();
	uint16_t crc = ::Calculate_CRC ( this->getWholeData(), full_size - CRC_SIZE );
	uint32_t size = this->getSourceDataSize();
	this->getSourceData()[ size ] = ( crc & 0XFF00) >>  8;	// CRCH
	this->getSourceData()[ size + 1 ] = ( crc ) & 0X00FF; 		// CRCL
}

void TmPacketBase::setData(const uint8_t* p_Data) {
	this->SpacePacketBase::setData( p_Data );
	this->tm_data = (TmPacketPointer*)p_Data;
}

void TmPacketBase::print() {
	/*uint8_t * wholeData = this->getWholeData();
	debug << "TmPacket contains: " << std::endl;
	for (uint8_t count = 0; count < this->getFullSize(); ++count ) {
		debug << std::hex << (uint16_t)wholeData[count] << " ";
	}
	debug << std::dec << std::endl;*/
}

bool TmPacketBase::checkAndSetStamper() {
	if (timeStamper == NULL) {
		//TODO: Adjust name?
		timeStamper = objectManager->get<TimeStamperIF>( objects::TIME_MANAGER );
		if ( timeStamper == NULL ) {
			error << "TmPacketBase::checkAndSetStamper: Stamper not found!" << std::endl;
			return false;
		}
	}
	return true;
}

TimeStamperIF* TmPacketBase::timeStamper = NULL;
