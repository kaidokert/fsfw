/**
 * @file CommunicationMessage.h
 *
 * @date 28.02.2020
 */

#ifndef FRAMEWORK_DEVICEHANDLERS_COMMUNICATIONMESSAGE_H_
#define FRAMEWORK_DEVICEHANDLERS_COMMUNICATIONMESSAGE_H_
#include "../devicehandlers/CommunicationMessage.h"

#include "../ipc/MessageQueueMessage.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../devicehandlers/DeviceHandlerBase.h"

/**
 * @brief Message type to send larger messages
 *
 * @details
 * Can be used to pass information like data pointers and
 * data sizes between communication tasks.
 *
 */
class CommunicationMessage: public MessageQueueMessage {
public:
	enum messageType {
		NONE,
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
	virtual ~CommunicationMessage();

	/**
	 * Message Type is stored as the fifth byte of the message data
	 * @param status
	 */
	void setMessageType(messageType status);
	messageType getMessageType() const;

	/**
	 * This is a unique ID which can be used to handle different kinds of messages.
	 * For example, the same interface (e.g. SPI) could be used to exchange raw data
	 * (e.g. sensor values) and data stored in the IPC store.
	 * The ID can be used to distinguish the messages in child implementations.
	 * The message ID is stored as the sixth byte of the message data.
	 * @param messageId
	 */
	void setMessageId(uint8_t messageId);
	uint8_t getMessageId() const;

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
	void setSendRequestRaw(uint32_t address, uint32_t length,
			uint16_t sendBufferPosition = 0);

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

	/**
	 * First four bytes of message data
	 * @param address
	 */
	void setAddress(address_t address);

	address_t getAddress() const;
	/**
	 * Set byte as position of 4 byte data field
	 * @param byte
	 * @param position Position, 0 to 3 possible
	 */
	void setDataByte(uint8_t byte, uint8_t position);
	uint8_t getDataByte(uint8_t position) const;

	/**
	 * Set 2 byte value at position 1 or 2 of data field
	 * @param data
	 * @param position 0 or 1 possible
	 */
	void setDataUint16(uint16_t data, uint8_t position);
	uint16_t getDataUint16(uint8_t position) const;

	void setUint32Data(uint32_t data);
	uint32_t getUint32Data() const;

	/**
	 * Stored in Bytes 13-16 of message data
	 * @param length
	 */
	void setDataLen(uint32_t length);

	uint32_t getDataLen() const;

	/**
	 * Stored in last four bytes (Bytes 17-20) of message data
	 * @param sendData
	 */
	void setDataPointer(const void * data);

	/**
	 * In case the send request data or reply data is to be stored in a buffer,
	 * a buffer Position can be stored here as the seventh and eigth byte of
	 * the message, so the receive buffer can't be larger than sizeof(uint16_t) for now.
	 * @param bufferPosition In case the data is stored in a buffer, the position can be supplied here
	 */
	void setBufferPosition(uint16_t bufferPosition);
	uint16_t getBufferPosition() const;
	void setStoreId(store_address_t storeId);
	store_address_t getStoreId() const;

	/**
	 * Clear the message. Deletes IPC Store data
	 * and sets all data to 0. Also sets message type to NONE
	 */
	void clearCommunicationMessage();
private:
	bool uninitialized; //!< Could be used to warn if data has not been set.
};

#endif /* FRAMEWORK_DEVICEHANDLERS_COMMUNICATIONMESSAGE_H_ */
