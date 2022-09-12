#include "CfdpMessage.h"

CfdpMessage::CfdpMessage() = default;

CfdpMessage::~CfdpMessage() = default;

void CfdpMessage::setCommand(CommandMessage *message, store_address_t cfdpPacket) {
  message->setParameter(cfdpPacket.raw);
}

store_address_t CfdpMessage::getStoreId(const CommandMessage *message) {
  store_address_t storeId;
  storeId = static_cast<store_address_t>(message->getParameter());
  return storeId;
}

void CfdpMessage::clear(CommandMessage *message) {}
