/**
 * @file CommunicationMessage.h
 *
 * @date 28.02.2020
 */

#ifndef FRAMEWORK_DEVICEHANDLERS_COMMUNICATIONMESSAGE_H_
#define FRAMEWORK_DEVICEHANDLERS_COMMUNICATIONMESSAGE_H_
#include <framework/devicehandlers/CommunicationMessage.h>

#include <framework/ipc/MessageQueueMessage.h>
#include <framework/storagemanager/StorageManagerIF.h>

/**
 * @brief Used to pass communication information between tasks
 *
 * @details
 * Can be used to pass information like data pointers and
 * data sizes between communication tasks
 * like the Device Handler Comm Interfaces and Polling Tasks.
 *
 * Can also be used to exchange actual data if its not too large
 * (e.g. Sensor values).
 *
 */
class CommunicationMessage: public MessageQueueMessage {
public:
	enum messageType {
		SEND_DATA_FROM_POINTER,
		SEND_DATA_FROM_IPC_STORE,
		SEND_DATA_RAW,
		REPLY_DATA_FROM_POINTER,
		REPLY_DATA_IPC_STORE,
		REPLY_DATA_RAW,
		FAULTY,
	};

	//Add other messageIDs here if necessary.
	static const uint8_t COMMUNICATION_MESSAGE_SIZE = HEADER_SIZE + 4 * sizeof(uint32_t);

	CommunicationMessage();

	/**
	 * Send requests with pointer to the data to be sent and send data length
	 * @param address Target Address, first four bytes
	 * @param dataLen Length of data to send, next four bytes
	 * @param data Pointer to data to send
	 *
	 */
	void setSendRequestFromPointer(uint32_t address, uint32_t dataLen, const uint8_t * data);

	/**
	 * Send requests with a store ID, using the IPC store
	 * @param address Target Address, first four bytes
	 * @param storeId Store ID in the IPC store
	 *
	 */
	void setSendRequestFromIpcStore(uint32_t address, store_address_t storeId);

	/**
	 * Send requests with data length and data in message (max. 4 bytes)
	 * @param address Target Address, first four bytes
	 * @param dataLen Length of data to send, next four bytes
	 * @param data Pointer to data to send
	 *
	 */
	void setSendRequestRaw(uint32_t address);

	/**
	 * Data message with data stored in IPC store
	 * @param address Target Address, first four bytes
	 * @param length
	 * @param storeId
	 */
	void setDataReplyFromIpcStore(uint32_t address, store_address_t storeId);

	/**
	 * Data reply with data stored in buffer, passing the pointer to
	 * the buffer and the data size
	 * @param address Target Address, first four bytes
	 * @param dataLen Length of data to send, next four bytes
	 * @param data Pointer to the data
	 */
	void setDataReplyFromPointer(uint32_t address, uint32_t dataLen, uint8_t * data);

	/**
	 * Data message with data stored in actual message.
	 * 4 byte datafield is intialized with 0.
	 * Set data with specific setter functions below.
	 * Can also be used to supply information at which position the raw data should be stored
	 * in a receive buffer.
	 */
	void setDataReplyRaw(uint32_t address, uint32_t length, uint16_t receiveBufferPosition = 0);

	/*
	 * The following functions can be used to
	 * set the data field (4 bytes possible);
	 */
	void setDataByte1(uint8_t byte1);
	void setDataByte2(uint8_t byte2);
	void setDataByte3(uint8_t byte3);
	void setDataByte4(uint8_t byte4);

	void setDataUINT16_1(uint16_t data1);
	void setDataUINT16_2(uint16_t data2);

	void setData(uint32_t data);

private:
	/**
	 * Message Type is stored as the fifth byte of the message data
	 * @param status
	 */
	void setMessageType(messageType status);

	/**
	 * First four bytes of message data
	 * @param address
	 */
	void setAddress(uint32_t address);

	/**
	 * Stored in Bytes 13-16 of message data
	 * @param length
	 */
	void setDataLen(uint32_t length);

	/**
	 * Stored in last four bytes (Bytes 17-20) of message data
	 * @param sendData
	 */
	void setDataPointer(const uint8_t * sendData);

	/**
	 * Buffer Position is stored as the seventh and eigth byte of
	 * the message, so the receive buffer can't be larger than sizeof(uint16_t) for now
	 * @param bufferPosition
	 */
	void setReceiveBufferPosition(uint16_t bufferPosition);
	void setStoreId(store_address_t storeId);

	virtual ~CommunicationMessage();

	bool uninitialized;
};

#endif /* FRAMEWORK_DEVICEHANDLERS_COMMUNICATIONMESSAGE_H_ */
