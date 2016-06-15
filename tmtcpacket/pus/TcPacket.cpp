/*
 * TcPacket.cpp
 *
 *  Created on: 18.06.2012
 *      Author: baetz
 */

#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tmtcpacket/pus/TcPacket.h>
#include <string.h>
template <uint32_t byte_size>
TcPacket<byte_size>::TcPacket() : TcPacketBase( (uint8_t*)&this->local_data ) {
	memset( &this->local_data, 0, sizeof(this->local_data) );
	//Set all constant elements in header according to ECSS E-70-41A (2003)
	//Primary header:
	//Set APID to idle packet
	this->local_data.primary.packet_id_h = 0b00011000;
	this->setAPID( APID_IDLE_PACKET );
	//Set Sequence Flags to "stand-alone packet"
	this->local_data.primary.sequence_control_h = 0b11000000;
	//Set packet size to size of data field header + CRC
	this->setPacketDataLength( sizeof(this->local_data.data_field) + CRC_SIZE );

	//Data Field Header:
	//Set CCSDS_secondary_header_flag to 0, version number to 001 and ack to 0000
	this->local_data.data_field.version_type_ack = 0b00010000;

}

template <uint32_t byte_size>
TcPacket<byte_size>::TcPacket( uint16_t new_apid, uint8_t new_ack, uint8_t new_service, uint8_t new_subservice, uint16_t new_sequence_count)  : TcPacketBase( (uint8_t*)&this->local_data ) {
	memset( &this->local_data, 0, sizeof(this->local_data) );
	//Set all constant elements in header according to ECSS E-70-41A (2003)
	//Primary header:
	//Set APID to idle packet
	this->local_data.primary.packet_id_h = 0b00011000;
	this->setAPID( new_apid );
	//Set Sequence Flags to "stand-alone packet"
	this->local_data.primary.sequence_control_h = 0b11000000;
	this->setPacketSequenceCount( new_sequence_count );
	this->setPacketDataLength( sizeof(this->local_data.data_field) + CRC_SIZE );

	//Data Field Header:
	//Set CCSDS_secondary_header_flag to 0, version number to 001 and ack to 0000
	this->local_data.data_field.version_type_ack = 0b00010000;
	this->local_data.data_field.version_type_ack |= ( new_ack & 0x0F);
	this->local_data.data_field.service_type = new_service;
	this->local_data.data_field.service_subtype = new_subservice;
	this->setErrorControl();

}

template <uint32_t byte_size>
TcPacket< byte_size >::~TcPacket() {
}

template <uint32_t byte_size>
bool TcPacket<byte_size>::addApplicationData( uint8_t* newData, uint32_t amount ) {
	if ( amount <= ( sizeof(this->local_data.application_data) - CRC_SIZE ) ) {
		memcpy( this->local_data.application_data, newData, amount );
		this->setPacketDataLength( amount + sizeof(this->local_data.data_field) + CRC_SIZE  - 1 );
		this->setErrorControl();
		return true;
	} else {
		return false;
	}
}

template class TcPacket<TC_PACKET_MIN_SIZE>;
template class TcPacket<32>;
template class TcPacket<64>;
template class TcPacket<128>;
template class TcPacket<256>;
template class TcPacket<1024>;
