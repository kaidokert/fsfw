/*
 * TcPacketBase.cpp
 *
 *  Created on: 18.06.2012
 *      Author: baetz
 */

#include <framework/globalfunctions/crc_ccitt.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tmtcpacket/pus/TcPacketBase.h>

TcPacketBase::TcPacketBase( const uint8_t* set_data ) : SpacePacketBase( set_data ) {
	this->tc_data = (TcPacketPointer*)set_data;
}

TcPacketBase::~TcPacketBase() {
	//Nothing to do.
}

uint8_t TcPacketBase::getService() {
	return this->tc_data->data_field.service_type;
}

uint8_t TcPacketBase::getSubService() {
	return this->tc_data->data_field.service_subtype;
}

uint8_t TcPacketBase::getAcknowledgeFlags() {
	return this->tc_data->data_field.version_type_ack & 0b00001111;
}

const uint8_t* TcPacketBase::getApplicationData() const {
	return &this->tc_data->data;
}

uint16_t TcPacketBase::getApplicationDataSize() {
	return this->getPacketDataLength() - sizeof(this->tc_data->data_field)  - CRC_SIZE + 1;
}

uint16_t TcPacketBase::getErrorControl() {
	uint16_t size = this->getApplicationDataSize() + CRC_SIZE;
	uint8_t* p_to_buffer = &this->tc_data->data;
	return ( p_to_buffer[size - 2] << 8 ) + p_to_buffer[size - 1];
}

void TcPacketBase::setErrorControl() {
	uint32_t full_size = this->getFullSize();
	uint16_t crc = ::Calculate_CRC ( this->getWholeData(), full_size - CRC_SIZE );
	uint32_t size = this->getApplicationDataSize();
	(&tc_data->data)[ size ] = ( crc & 0XFF00) >>  8;	// CRCH
	(&tc_data->data)[ size + 1 ] = ( crc ) & 0X00FF; 		// CRCL
}

void TcPacketBase::setData(const uint8_t* p_Data) {
	this->SpacePacketBase::setData( p_Data );
	this->tc_data = (TcPacketPointer*)p_Data;
}

uint8_t TcPacketBase::getSecondaryHeaderFlag() {
	return (this->tc_data->data_field.version_type_ack & 0b10000000) >> 7;
}

uint8_t TcPacketBase::getPusVersionNumber() {
	return (this->tc_data->data_field.version_type_ack & 0b01110000) >> 4;
}

void TcPacketBase::print() {
	uint8_t * wholeData = this->getWholeData();
	debug << "TcPacket contains: " << std::endl;
	for (uint8_t count = 0; count < this->getFullSize(); ++count ) {
		debug << std::hex << (uint16_t)wholeData[count] << " ";
	}
	debug << std::dec << std::endl;
}
