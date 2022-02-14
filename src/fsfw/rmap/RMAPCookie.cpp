#include "fsfw/rmap/RMAPCookie.h"

#include <cstddef>

#include "fsfw/rmap/RMAPChannelIF.h"

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
  this->channel = NULL;
  this->command_mask = 0;

  this->dataCRC = 0;

  this->maxReplyLen = 0;
}

RMAPCookie::RMAPCookie(uint32_t set_address, uint8_t set_extended_address,
                       RMAPChannelIF *set_channel, uint8_t set_command_mask, size_t maxReplyLen) {
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
  this->channel = set_channel;
  this->command_mask = set_command_mask;
  this->dataCRC = 0;

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

void RMAPCookie::setChannel(RMAPChannelIF *channel) { this->channel = channel; }

void RMAPCookie::setCommandMask(uint8_t commandMask) { this->command_mask = commandMask; }

uint32_t RMAPCookie::getAddress() {
  return (header.address_hh << 24) + (header.address_h << 16) + (header.address_l << 8) +
         (header.address_ll);
}

uint8_t RMAPCookie::getExtendedAddress() { return header.extended_address; }

RMAPChannelIF *RMAPCookie::getChannel() { return channel; }

uint8_t RMAPCookie::getCommandMask() { return command_mask; }

RMAPCookie::~RMAPCookie() {}

size_t RMAPCookie::getMaxReplyLen() const { return maxReplyLen; }

void RMAPCookie::setMaxReplyLen(size_t maxReplyLen) { this->maxReplyLen = maxReplyLen; }

RMAPStructs::rmap_cmd_header *RMAPCookie::getHeader() { return &this->header; }

uint16_t RMAPCookie::getTransactionIdentifier() const {
  return static_cast<uint16_t>((header.tid_h << 8) | (header.tid_l));
}

void RMAPCookie::setTransactionIdentifier(uint16_t id_) {
  header.tid_l = id_ & 0xFF;
  header.tid_h = (id_ >> 8) & 0xFF;
}

uint32_t RMAPCookie::getDataLength() const {
  return static_cast<uint32_t>(header.datalen_h << 16 | header.datalen_m << 8 | header.datalen_l);
}
void RMAPCookie::setDataLength(uint32_t length_) {
  header.datalen_l = length_ & 0xff;
  header.datalen_m = (length_ >> 8) & 0xff;
  header.datalen_h = (length_ >> 16) & 0xff;
}
