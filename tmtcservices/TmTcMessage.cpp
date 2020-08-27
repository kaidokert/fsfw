#include "TmTcMessage.h"
#include <string.h>


TmTcMessage::TmTcMessage() {
	this->messageSize += sizeof(store_address_t);
}

TmTcMessage::~TmTcMessage() {
}

store_address_t TmTcMessage::getStorageId() {
	store_address_t temp_id;
	memcpy(&temp_id, this->getData(), sizeof(store_address_t) );
	return temp_id;
}

TmTcMessage::TmTcMessage(store_address_t store_id) {
	this->messageSize += sizeof(store_address_t);
	this->setStorageId(store_id);
}

size_t TmTcMessage::getMinimumMessageSize() {
	return this->HEADER_SIZE + sizeof(store_address_t);
}

void TmTcMessage::setStorageId(store_address_t store_id) {
	memcpy(this->getData(), &store_id, sizeof(store_address_t) );
}
