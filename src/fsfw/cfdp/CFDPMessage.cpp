#include "CFDPMessage.h"

CFDPMessage::CFDPMessage() {}

CFDPMessage::~CFDPMessage() {}

void CFDPMessage::setCommand(CommandMessage *message, store_address_t cfdpPacket) {
  message->setParameter(cfdpPacket.raw);
}

store_address_t CFDPMessage::getStoreId(const CommandMessage *message) {
  store_address_t storeAddressCFDPPacket;
  storeAddressCFDPPacket = static_cast<store_address_t>(message->getParameter());
  return storeAddressCFDPPacket;
}

void CFDPMessage::clear(CommandMessage *message) {}
