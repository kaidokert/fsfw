#ifndef FRAMEWORK_IPC_MESSAGEQUEUEMESSAGEIF_H_
#define FRAMEWORK_IPC_MESSAGEQUEUEMESSAGEIF_H_
#include <cstddef>
#include <cstdint>

/*
 * TODO: Actually, the definition of this ID to be a uint32_t is not  ideal and
 * breaks layering. However, it is difficult to keep layering, as the ID is
 * stored in many places and sent around in MessageQueueMessage.
 * Ideally, one would use the (current) object_id_t only, however, doing a
 * lookup of queueIDs for every call does not sound ideal.
 * In a first step, I'll circumvent the issue by not touching it,
 * maybe in a second step. This also influences Interface design
 * (getCommandQueue) and some other issues..
 */

typedef uint32_t MessageQueueId_t;

class MessageQueueMessageIF {
public:
	static const MessageQueueId_t NO_QUEUE = 0xffffffff;

	virtual ~MessageQueueMessageIF() {};

	/**
	 * @brief	With this method, the whole content and the message
	 * 			size is set to zero. Implementations should also take care
	 * 			to clear data which is stored indirectly (e.g. storage data).
	 */
	virtual void clear() = 0;

	/**
	 * @brief	Get read-only pointer to the complete data of the message.
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
	 * @brief	This method is used to extract the sender's message queue id
	 * 			information from a received message.
	 */
	virtual MessageQueueId_t getSender() const = 0;

	/**
	 * @brief	This method is used to fetch the data content of the message.
	 * @details
	 * It shall be used by child classes to add data at the right position.
	 */
	virtual const uint8_t* getData() const = 0;
	/**
	 * @brief	This method is used to fetch the data content of the message.
	 * @details
	 * It shall be used by child classes to add data at the right position.
	 */
	virtual uint8_t* getData() = 0;

	/**
	 * Get constant message size of current message implementation.
	 * @return
	 */
	virtual size_t getMessageSize() const = 0;

};



#endif /* FRAMEWORK_IPC_MESSAGEQUEUEMESSAGEIF_H_ */
