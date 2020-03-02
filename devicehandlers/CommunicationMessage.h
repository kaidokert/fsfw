/**
 * @file CommunicationMessage.h
 *
 * @date 28.02.2020
 */

#ifndef FRAMEWORK_DEVICEHANDLERS_COMMUNICATIONMESSAGE_H_
#define FRAMEWORK_DEVICEHANDLERS_COMMUNICATIONMESSAGE_H_
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
	enum communicationStatus {
		SEND_DATA,
		PROCESS_DATA,
		FAULTY,
	};

	//Add other messageIDs here if necessary.
	static const uint8_t COMMUNICATION_MESSAGE_SIZE = HEADER_SIZE + 4 * sizeof(uint32_t);

	CommunicationMessage();

	/**
	 * Send requests with pointer to the data to be sent and send data length
	 * @param address Target Address
	 * @param sendData
	 * @param length
	 */
	void setSendRequest(uint32_t address, const uint8_t * sendData, uint32_t length);

	/**
	 * Data message with data stored in IPC store
	 * @param address
	 * @param length
	 * @param storeId
	 */
	void setDataMessage(uint32_t address, uint32_t length, store_address_t * storeId);

	/**
	 * Data message with data stored in actual message.
	 * 4 byte datafield is intialized with 0.
	 * Set data with specific setter functions.
	 */
	void setDataMessage(uint32_t address, uint32_t length);

private:
	void setCommunicationStatus(communicationStatus status);
	void setAddress(uint32_t address);
	void setSendData(const uint8_t * sendData);
	void setDataLen(uint32_t length);

	/** For low size data, maximum of 4 bytes can be sent */
	void setDataByte1(uint8_t byte1);
	void setDataByte2(uint8_t byte2);
	void setDataByte3(uint8_t byte3);
	void setDataByte4(uint8_t byte4);

	void setDataUINT16_1(uint16_t data1);
	void setDataUINT16_2(uint16_t data2);

	void setData(uint32_t data);

	virtual ~CommunicationMessage();
};



#endif /* FRAMEWORK_DEVICEHANDLERS_COMMUNICATIONMESSAGE_H_ */
