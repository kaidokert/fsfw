/**
 * @file CommunicationMessage.cpp
 *
 * @date 28.02.2020
 */

#include "../devicehandlers/CommunicationMessage.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include <cstring>

CommunicationMessage::CommunicationMessage(): uninitialized(true) {
}

CommunicationMessage::~CommunicationMessage() {}

void CommunicationMessage::setSendRequestFromPointer(uint32_t address,
		uint32_t dataLen, const uint8_t * data) {
	setMessageType(SEND_DATA_FROM_POINTER);
	setAddress(address);
	setDataLen(dataLen);
	setDataPointer(data);
}

void CommunicationMessage::setSendRequestFromIpcStore(uint32_t address, store_address_t storeId) {
	setMessageType(SEND_DATA_FROM_IPC_STORE);
	setAddress(address);
	setStoreId(storeId.raw);
}

void CommunicationMessage::setSendRequestRaw(uint32_t address, uint32_t length,
		uint16_t sendBufferPosition) {
	setMessageType(SEND_DATA_RAW);
	setAddress(address);
	setDataLen(length);
	if(sendBufferPosition != 0) {
		setBufferPosition(sendBufferPosition);
	}
}

void CommunicationMessage::setDataReplyFromIpcStore(uint32_t address, store_address_t storeId) {
	setMessageType(REPLY_DATA_IPC_STORE);
	setAddress(address);
	setStoreId(storeId.raw);
}
void CommunicationMessage::setDataReplyFromPointer(uint32_t address,
		uint32_t dataLen, uint8_t *data) {
	setMessageType(REPLY_DATA_FROM_POINTER);
	setAddress(address);
	setDataLen(dataLen);
	setDataPointer(data);
}

void CommunicationMessage::setDataReplyRaw(uint32_t address,
		uint32_t length, uint16_t receiveBufferPosition) {
	setMessageType(REPLY_DATA_RAW);
	setAddress(address);
	setDataLen(length);
	if(receiveBufferPosition != 0) {
		setBufferPosition(receiveBufferPosition);
	}
}

void CommunicationMessage::setMessageType(messageType status) {
	uint8_t status_uint8 = status;
	memcpy(getData() + sizeof(uint32_t), &status_uint8, sizeof(status_uint8));
}

void CommunicationMessage::setAddress(address_t address) {
	memcpy(getData(),&address,sizeof(address));
}

address_t CommunicationMessage::getAddress() const {
	address_t address;
	memcpy(&address,getData(),sizeof(address));
	return address;
}

void CommunicationMessage::setBufferPosition(uint16_t bufferPosition) {
	memcpy(getData() + sizeof(uint32_t) + sizeof(uint16_t),
			&bufferPosition, sizeof(bufferPosition));
}

uint16_t CommunicationMessage::getBufferPosition() const {
	uint16_t bufferPosition;
	memcpy(&bufferPosition,
			getData() + sizeof(uint32_t) + sizeof(uint16_t), sizeof(bufferPosition));
	return bufferPosition;
}

void CommunicationMessage::setDataPointer(const void * data) {
	memcpy(getData() + 3 * sizeof(uint32_t), &data, sizeof(uint32_t));
}

void CommunicationMessage::setStoreId(store_address_t storeId) {
	memcpy(getData() + 2 * sizeof(uint32_t), &storeId.raw, sizeof(uint32_t));
}

store_address_t CommunicationMessage::getStoreId() const{
	store_address_t temp;
	memcpy(&temp.raw,getData() + 2 * sizeof(uint32_t), sizeof(uint32_t));
	return temp;
}

void CommunicationMessage::setDataLen(uint32_t length) {
	memcpy(getData() + 2 * sizeof(uint32_t), &length, sizeof(length));
}

uint32_t CommunicationMessage::getDataLen() const {
	uint32_t len;
	memcpy(&len, getData() + 2 * sizeof(uint32_t), sizeof(len));
	return len;
}

void CommunicationMessage::setUint32Data(uint32_t data) {
	memcpy(getData() + 3 * sizeof(uint32_t), &data, sizeof(data));
}

uint32_t CommunicationMessage::getUint32Data() const{
	uint32_t data;
	memcpy(&data,getData() + 3 * sizeof(uint32_t),  sizeof(data));
	return data;
}

void CommunicationMessage::setDataByte(uint8_t byte, uint8_t position) {
	if(0 <= position && position <= 3) {
		memcpy(getData() + 3 * sizeof(uint32_t) + position * sizeof(uint8_t), &byte, sizeof(byte));
	}
	else {
		sif::error << "Comm Message: Invalid byte position" << std::endl;
	}
}

uint8_t CommunicationMessage::getDataByte(uint8_t position) const {
	if(0 <= position && position <= 3) {
		uint8_t byte;
		memcpy(&byte, getData() + 3 * sizeof(uint32_t) + position * sizeof(uint8_t), sizeof(byte));
		return byte;
	}
	else {
		return 0;
		sif::error << "Comm Message: Invalid byte position" << std::endl;
	}
}

void CommunicationMessage::setDataUint16(uint16_t data, uint8_t position) {
	if(position == 0 || position == 1) {
		memcpy(getData() + 3 * sizeof(uint32_t) + position * sizeof(uint16_t), &data, sizeof(data));
	}
	else {
		sif::error << "Comm Message: Invalid byte position" << std::endl;
	}

}

uint16_t CommunicationMessage::getDataUint16(uint8_t position) const{
	if(position == 0 || position == 1) {
		uint16_t data;
		memcpy(&data, getData() + 3 * sizeof(uint32_t) + position * sizeof(uint16_t), sizeof(data));
		return data;
	}
	else {
		return 0;
		sif::error << "Comm Message: Invalid byte position" << std::endl;
	}
}

CommunicationMessage::messageType CommunicationMessage::getMessageType() const{
	messageType messageType;
	memcpy(&messageType, getData() + sizeof(uint32_t),sizeof(uint8_t));
	return messageType;
}

void CommunicationMessage::setMessageId(uint8_t messageId) {
	memcpy(getData() + sizeof(uint32_t) + sizeof(uint8_t), &messageId, sizeof(messageId));
}

uint8_t CommunicationMessage::getMessageId() const {
	uint8_t messageId;
	memcpy(&messageId, getData() + sizeof(uint32_t) + sizeof(uint8_t), sizeof(messageId));
	return messageId;
}

void CommunicationMessage::clearCommunicationMessage() {
	messageType messageType = getMessageType();
	switch(messageType) {
	case(messageType::REPLY_DATA_IPC_STORE):
	case(messageType::SEND_DATA_FROM_IPC_STORE): {
		store_address_t storeId = getStoreId();
		StorageManagerIF *ipcStore = objectManager->
				get<StorageManagerIF>(objects::IPC_STORE);
		if (ipcStore != NULL) {
			ipcStore->deleteData(storeId);
		}
	}
	/* NO BREAK falls through*/
	default:
		memset(getData(),0,4*sizeof(uint32_t));
		break;
	}
}

