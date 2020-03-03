/**
 * @file CommunicationMessage.cpp
 *
 * @date 28.02.2020
 */

#include <framework/devicehandlers/CommunicationMessage.h>
#include <cstring>

CommunicationMessage::CommunicationMessage(): uninitialized(true) {
}

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
	setStoreId(storeId);
}

void CommunicationMessage::setSendRequestRaw(uint32_t address) {
}

void CommunicationMessage::setDataReplyFromIpcStore(uint32_t address, store_address_t storeId) {
	setMessageType(REPLY_DATA_IPC_STORE);
	setAddress(address);
	setStoreId(storeId);
}
void CommunicationMessage::setDataReplyFromPointer(uint32_t address,
		uint32_t dataLen, uint8_t *data) {
}

void CommunicationMessage::setDataReplyRaw(uint32_t address,
		uint32_t length, uint16_t receiveBufferPosition) {
	setMessageType(REPLY_DATA_RAW);
	setAddress(address);
	setDataLen(length);
	if(receiveBufferPosition != 0) {
		setReceiveBufferPosition(receiveBufferPosition);
	}
}



void CommunicationMessage::setMessageType(messageType status) {
	uint8_t status_uint8 = status;
	memcpy(getData() + sizeof(uint32_t), &status_uint8, sizeof(status_uint8));
}

void CommunicationMessage::setAddress(uint32_t address) {
	memcpy(getData(),&address,sizeof(address));
}

void CommunicationMessage::setReceiveBufferPosition(uint16_t bufferPosition) {
	memcpy(getData() + sizeof(uint32_t) + sizeof(uint8_t),
			&bufferPosition, sizeof(bufferPosition));
}

void CommunicationMessage::setDataPointer(const uint8_t *sendData) {
	memcpy(getData() + 3 * sizeof(uint32_t), &sendData, sizeof(uint32_t));
}

void CommunicationMessage::setStoreId(store_address_t storeId) {
	memcpy(getData() + 2 * sizeof(uint32_t), &storeId, sizeof(store_address_t));
}

void CommunicationMessage::setDataLen(uint32_t length) {
	memcpy(getData() + 2 * sizeof(uint32_t), &length, sizeof(length));
}

void CommunicationMessage::setDataByte1(uint8_t byte1) {
}

void CommunicationMessage::setDataByte2(uint8_t byte2) {
}

void CommunicationMessage::setDataByte3(uint8_t byte3) {
}

void CommunicationMessage::setDataByte4(uint8_t byte4) {
}

void CommunicationMessage::setDataUINT16_1(uint16_t data1) {
}

void CommunicationMessage::setDataUINT16_2(uint16_t data2) {
}

void CommunicationMessage::setData(uint32_t data) {
}

CommunicationMessage::~CommunicationMessage() {
}





