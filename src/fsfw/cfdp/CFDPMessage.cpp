#include "CFDPMessage.h"

CFDPMessage::CFDPMessage() {}

CFDPMessage::~CFDPMessage() {}

void CFDPMessage::setCommand(CommandMessage *message, store_address_t cfdpPacket) {
  message->setParameter(cfdpPacket.raw);
}

store_address_t CFDPMessage::getStoreId(const CommandMessage *message) {
  store_address_t storeAddressCFDPPacket;
  storeAddressCFDPPacket = message->getParameter();
  return storeAddressCFDPPacket;
}

void CFDPMessage::clear(CommandMessage *message) {}
