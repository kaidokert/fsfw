#include "SpacePacketBase.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include <cstring>

SpacePacketBase::SpacePacketBase( const uint8_t* set_address ) {
	this->data = (SpacePacketPointer*) set_address;
}

SpacePacketBase::~SpacePacketBase() {
};

//CCSDS Methods:
uint8_t SpacePacketBase::getPacketVersionNumber( void ) {
	return (this->data->header.packet_id_h & 0b11100000) >> 5;
}

void SpacePacketBase::initSpacePacketHeader(bool isTelecommand,
		bool hasSecondaryHeader, uint16_t apid, uint16_t sequenceCount) {
	//reset header to zero:
	memset(data,0, sizeof(this->data->header) );
	//Set TC/TM bit.
	data->header.packet_id_h = ((isTelecommand? 1 : 0)) << 4;
	//Set secondaryHeader bit
	data->header.packet_id_h |= ((hasSecondaryHeader? 1 : 0)) << 3;
	this->setAPID( apid );
	//Always initialize as standalone packets.
	data->header.sequence_control_h = 0b11000000;
	setPacketSequenceCount(sequenceCount);

}

bool SpacePacketBase::isTelecommand( void ) {
	return (this->data->header.packet_id_h & 0b00010000) >> 4;
}

bool SpacePacketBase::hasSecondaryHeader( void ) {
	return (this->data->header.packet_id_h & 0b00001000) >> 3;
}

uint16_t SpacePacketBase::getPacketId() {
	return ( (this->data->header.packet_id_h) << 8 ) +
			this->data->header.packet_id_l;
}

uint16_t SpacePacketBase::getAPID( void ) const {
	return ( (this->data->header.packet_id_h & 0b00000111) << 8 ) +
			this->data->header.packet_id_l;
}

void SpacePacketBase::setAPID( uint16_t new_apid ) {
	//Use first three bits of new APID, but keep rest of packet id as it was (see specification).
	this->data->header.packet_id_h = (this->data->header.packet_id_h & 0b11111000) | ( ( new_apid & 0x0700 ) >> 8 );
	this->data->header.packet_id_l = ( new_apid & 0x00FF );
}

uint16_t SpacePacketBase::getPacketSequenceControl( void ) {
	return ( (this->data->header.sequence_control_h) << 8 )
		+ this->data->header.sequence_control_l;
}

uint8_t SpacePacketBase::getSequenceFlags( void ) {
	return (this->data->header.sequence_control_h & 0b11000000) >> 6 ;
}

uint16_t SpacePacketBase::getPacketSequenceCount( void ) const {
	return ( (this->data->header.sequence_control_h & 0b00111111) << 8 )
		+ this->data->header.sequence_control_l;
}

void SpacePacketBase::setPacketSequenceCount( uint16_t new_count) {
	this->data->header.sequence_control_h = ( this->data->header.sequence_control_h & 0b11000000 ) | ( ( (new_count%LIMIT_SEQUENCE_COUNT) & 0x3F00 ) >> 8 );
	this->data->header.sequence_control_l = ( (new_count%LIMIT_SEQUENCE_COUNT) & 0x00FF );
}

uint16_t SpacePacketBase::getPacketDataLength( void ) {
	return ( (this->data->header.packet_length_h) << 8 )
		+ this->data->header.packet_length_l;
}

void SpacePacketBase::setPacketDataLength( uint16_t new_length) {
	this->data->header.packet_length_h = ( ( new_length & 0xFF00 ) >> 8 );
	this->data->header.packet_length_l = ( new_length & 0x00FF );
}

size_t SpacePacketBase::getFullSize() {
	//+1 is done because size in packet data length field is: size of data field -1
	return this->getPacketDataLength() + sizeof(this->data->header) + 1;
}

uint8_t* SpacePacketBase::getWholeData() {
	return (uint8_t*)this->data;
}

void SpacePacketBase::setData( const uint8_t* p_Data ) {
	this->data = (SpacePacketPointer*)p_Data;
}

uint32_t SpacePacketBase::getApidAndSequenceCount() const {
	return (getAPID() << 16) + getPacketSequenceCount();
}

uint8_t* SpacePacketBase::getPacketData() {
	return &(data->packet_data);
}
