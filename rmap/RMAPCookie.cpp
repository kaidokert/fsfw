/*
 * RMAPcpp
 *
 *  Created on: 07.11.2012
 *      Author: mohr
 */

#include <framework/rmap/RMAPChannelIF.h>
#include <framework/rmap/RMAPCookie.h>
#include <stddef.h>

//TODO use ctor initialization list
RMAPCookie::RMAPCookie() {
	this->header.dest_address = 0;
	this->header.protocol = 0x01;
	this->header.instruction = 0;
	this->header.dest_key = 0;
	this->header.source_address = 0;
	this->header.tid_h = 0;
	this->header.tid_l = 0;
	this->header.extended_address = 0;
	this->header.address_hh = 0;
	this->header.address_h = 0;
	this->header.address_l = 0;
	this->header.address_ll = 0;
	this->header.datalen_h = 0;
	this->header.datalen_m = 0;
	this->header.datalen_l = 0;
	this->header.header_crc = 0;


	this->txdesc = NULL;
	this->rxdesc_index = 0;
	this->channel = NULL;
	this->command_mask = 0;

	this->maxReplyLen = 0;
}



RMAPCookie::RMAPCookie(uint32_t set_address, uint8_t set_extended_address,
		RMAPChannelIF *set_channel, uint8_t set_command_mask, uint32_t maxReplyLen) {
	this->header.dest_address = 0;
	this->header.protocol = 0x01;
	this->header.instruction = 0;
	this->header.dest_key = 0;
	this->header.source_address = 0;
	this->header.tid_h = 0;
	this->header.tid_l = 0;
	this->header.extended_address = set_extended_address;
	setAddress(set_address);
	this->header.datalen_h = 0;
	this->header.datalen_m = 0;
	this->header.datalen_l = 0;
	this->header.header_crc = 0;

	this->txdesc = NULL;
	this->rxdesc_index = 0;
	this->channel = set_channel;
	this->command_mask = set_command_mask;

	this->maxReplyLen = maxReplyLen;
}


void RMAPCookie::setAddress(uint32_t address) {
	this->header.address_hh = (address & 0xFF000000) >> 24;
	this->header.address_h = (address & 0x00FF0000) >> 16;
	this->header.address_l = (address & 0x0000FF00) >> 8;
	this->header.address_ll = address & 0x000000FF;
}

void RMAPCookie::setExtendedAddress(uint8_t extendedAddress) {
	this->header.extended_address = extendedAddress;
}

void RMAPCookie::setChannel(RMAPChannelIF *channel) {
	this->channel = channel;
}

void RMAPCookie::setCommandMask(uint8_t commandMask) {
	this->command_mask = commandMask;
}

uint32_t RMAPCookie::getAddress() {
	return (header.address_hh << 24) + (header.address_h << 16)
			+ (header.address_l << 8) + (header.address_ll);
}

uint8_t RMAPCookie::getExtendedAddress() {
	return header.extended_address;
}

RMAPChannelIF *RMAPCookie::getChannel() {
	return channel;
}

uint8_t RMAPCookie::getCommandMask() {
	return command_mask;
}

RMAPCookie::~RMAPCookie() {

}

uint32_t RMAPCookie::getMaxReplyLen() const {
	return maxReplyLen;
}

void RMAPCookie::setMaxReplyLen(uint32_t maxReplyLen) {
	this->maxReplyLen = maxReplyLen;
}
