#include "fsfw/tmtcservices/TmTcMessage.h"

#include <cstring>

TmTcMessage::TmTcMessage() { this->messageSize += sizeof(store_address_t); }

TmTcMessage::~TmTcMessage() = default;

store_address_t TmTcMessage::getStorageId() {
  store_address_t temp_id;
  memcpy(&temp_id, this->getData(), sizeof(store_address_t));
  return temp_id;
}

TmTcMessage::TmTcMessage(store_address_t storeId) {
  this->messageSize += sizeof(store_address_t);
  this->setStorageId(storeId);
}

size_t TmTcMessage::getMinimumMessageSize() const {
  return TmTcMessage::HEADER_SIZE + sizeof(store_address_t);
}

void TmTcMessage::setStorageId(store_address_t storeId) {
  std::memcpy(this->getData(), &storeId, sizeof(store_address_t));
}
