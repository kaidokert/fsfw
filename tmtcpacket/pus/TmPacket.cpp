/*
 * TmPacket.cpp
 *
 *  Created on: 18.06.2012
 *      Author: baetz
 */

#include <framework/tmtcpacket/pus/TmPacket.h>
#include <string.h>
template <uint32_t byte_size>
TmPacket<byte_size>::TmPacket() : TmPacketBase( (uint8_t*)&this->local_data ) {
	memset( &this->local_data, 0, sizeof(this->local_data) );
	//Set all constant elements in header according to ECSS E-70-41A (2003)
	//Primary header:
	//Set APID to idle packet
	local_data.primary.packet_id_h = 0b00001000;
	setAPID( APID_IDLE_PACKET );
	//Set Sequence Flags to "stand-alone packet"
	local_data.primary.sequence_control_h = 0b11000000;
	//Set packet size to size of data field header + CRC
	setPacketDataLength( sizeof(this->local_data.data_field) + CRC_SIZE );
	if (checkAndSetStamper()) {
		timeStamper->addTimeStamp(local_data.data_field.time, sizeof(local_data.data_field.time));
	}
	//Data Field Header:
	//Set CCSDS_secondary_header_flag to 0, version number to 001 and ack to 0000
	this->local_data.data_field.version_type_ack = 0b00010000;

}

template <uint32_t byte_size>
TmPacket<byte_size>::TmPacket( uint16_t new_apid, uint8_t new_service, uint8_t new_subservice, uint8_t new_packet_counter )  : TmPacketBase( (uint8_t*)&this->local_data ) {
	memset( &this->local_data, 0, sizeof(this->local_data) );
	//Set all constant elements in header according to ECSS E-70-41A (2003)
	//Primary header:
	//Set APID to idle packet
	this->local_data.primary.packet_id_h = 0b00001000;
	this->setAPID( new_apid );
	//Set Sequence Flags to "stand-alone packet"
	this->local_data.primary.sequence_control_h = 0b11000000;
	//Set Data Length to minimum size (Data Field Header + CRC size)
	this->setPacketDataLength( sizeof(this->local_data.data_field) + CRC_SIZE );

	//Data Field Header:
	//Set CCSDS_secondary_header_flag to 0, version number to 001 and ack to 0000
	this->local_data.data_field.version_type_ack = 0b00010000;
	this->local_data.data_field.service_type = new_service;
	this->local_data.data_field.service_subtype = new_subservice;
	this->local_data.data_field.subcounter = new_packet_counter;

	if (checkAndSetStamper()) {
		timeStamper->addTimeStamp(local_data.data_field.time, sizeof(local_data.data_field.time));
	}

//	this->local_data.data_field.destination = new_destination;
	this->setErrorControl();
}

template <uint32_t byte_size>
TmPacket< byte_size >::~TmPacket() {
}

template <uint32_t byte_size>
bool TmPacket<byte_size>::addSourceData( uint8_t* newData, uint32_t amount ) {
	if ( amount <= ( sizeof(this->local_data.source_data) - CRC_SIZE ) ) {
		memcpy( this->local_data.source_data, newData, amount );
		this->setPacketDataLength( amount + sizeof(this->local_data.data_field) + CRC_SIZE  - 1 );
		this->setErrorControl();
		return true;
	} else {
		return false;
	}
}

template class TmPacket<TmPacketBase::TM_PACKET_MIN_SIZE>;
template class TmPacket<64>;
template class TmPacket<128>;
template class TmPacket<256>;
template class TmPacket<512>;
template class TmPacket<1024>;
