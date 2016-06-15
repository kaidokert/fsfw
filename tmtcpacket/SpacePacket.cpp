/*
 * SpacePacket.cpp
 *
 *  Created on: Mar 23, 2012
 *      Author: baetz
 */

#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tmtcpacket/ccsds_header.h>
#include <framework/tmtcpacket/SpacePacket.h>
#include <string.h>

SpacePacket::SpacePacket(  uint16_t set_packet_data_length, uint8_t set_type, uint16_t new_apid, uint16_t set_count  ):
SpacePacketBase( (uint8_t*)&this->local_data ) {
	//reset everything to zero:
	memset(this->local_data.byteStream,0 , sizeof(this->local_data.byteStream) );
	//Primary header:
	this->local_data.fields.header.packet_id_h = 0b00000000 + ( (set_type & 0b1) << 4 );
	this->setAPID( new_apid );
	this->local_data.fields.header.sequence_control_h = 0b11000000;
	this->setPacketSequenceCount(set_count);
	if ( set_packet_data_length <= sizeof(this->local_data.fields.buffer) ) {
		this->setPacketDataLength(set_packet_data_length);
	} else {
		this->setPacketDataLength( sizeof(this->local_data.fields.buffer) );
	}
}

SpacePacket::~SpacePacket( void ) {
}

bool SpacePacket::addWholeData( const uint8_t* p_Data, uint32_t packet_size ) {
	if ( packet_size <= sizeof(this->data) ) {
		memcpy( &this->local_data.byteStream, p_Data, packet_size );
		return true;
	} else {
		return false;
	}
}
