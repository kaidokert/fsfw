#ifndef FRAMEWORK_IPC_MESSAGEQUEUEMESSAGEIF_H_
#define FRAMEWORK_IPC_MESSAGEQUEUEMESSAGEIF_H_
#include <framework/ipc/MessageQueueSenderIF.h>
#include <cstddef>

class MessageQueueMessageIF {
public:
	virtual ~MessageQueueMessageIF() {};

	/**
	 * @brief	With this method, the whole content and the message
	 * 			size is set to zero.
	 */
	virtual void clear() = 0;
	/**
	 * @brief	This is a debug method that prints the content
	 * 			(till messageSize) to the debug output.
	 */
	virtual void print() = 0;

	/**
	 * @brief	Get read-only pointer to the raw buffer.
	 * @return
	 */
	virtual const uint8_t* getBuffer() const = 0;

	/**
	 * @brief	This method is used to get the complete data of the message.
	 */
	virtual uint8_t* getBuffer() = 0;

	/**
	 * @brief	This method is used to set the sender's message queue id
	 * 			information prior to sending the message.
	 * @param setId
	 * The message queue id that identifies the sending message queue.
	 */
	virtual void setSender(MessageQueueId_t setId) = 0;

	/**
	 * @brief	This helper function is used by the MessageQueue class to
	 * 			check the size of an incoming message.
	 */
	virtual size_t getMinimumMessageSize() = 0;
};



#endif /* FRAMEWORK_IPC_MESSAGEQUEUEMESSAGEIF_H_ */
