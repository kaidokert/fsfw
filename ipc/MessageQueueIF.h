#ifndef FRAMEWORK_IPC_MESSAGEQUEUEIF_H_
#define FRAMEWORK_IPC_MESSAGEQUEUEIF_H_

// COULDDO: We could support blocking calls

#include "MessageQueueMessage.h"
#include "MessageQueueSenderIF.h"
#include "../returnvalues/HasReturnvaluesIF.h"
class MessageQueueIF {
public:

	static const MessageQueueId_t NO_QUEUE = MessageQueueSenderIF::NO_QUEUE; //!< Ugly hack.

	static const uint8_t INTERFACE_ID = CLASS_ID::MESSAGE_QUEUE_IF;
	/**
	 * No new messages on the queue
	 */
	static const ReturnValue_t EMPTY = MAKE_RETURN_CODE(1);
	/**
	 * No space left for more messages
	 */
	static const ReturnValue_t FULL = MAKE_RETURN_CODE(2);
	/**
	 * Returned if a reply method was called without partner
	 */
	static const ReturnValue_t NO_REPLY_PARTNER = MAKE_RETURN_CODE(3);

	virtual ~MessageQueueIF() {}
	/**
	 * @brief	This operation sends a message to the last communication partner.
	 * @details	This operation simplifies answering an incoming message by using the stored
	 * 			lastParnter information as destination. If there was no message received yet
	 * 			(i.e. lastPartner is zero), an error code is returned.
	 * @param message	A pointer to a previously created message, which is sent.
	 * \return RETURN_OK if ok
	 * \return NO_REPLY_PARTNER Should return NO_REPLY_PARTNER if partner was found
	 */
	virtual ReturnValue_t reply( MessageQueueMessage* message ) = 0;

	/**
	 * @brief	This function reads available messages from the message queue and returns the sender.
	 * @details	It works identically to the other receiveMessage call, but in addition returns the
	 * 			sender's queue id.
	 * @param message	A pointer to a message in which the received data is stored.
	 * @param receivedFrom	A pointer to a queue id in which the sender's id is stored.
	 */
	virtual ReturnValue_t receiveMessage(MessageQueueMessage* message,
			MessageQueueId_t *receivedFrom) = 0;

	/**
	 * @brief	This function reads available messages from the message queue.
	 * @details	If data is available it is stored in the passed message pointer. The message's
	 * 			original content is overwritten and the sendFrom information is stored in the
	 * 			lastPartner attribute. Else, the lastPartner information remains untouched, the
	 * 			message's content is cleared and the function returns immediately.
	 * @param message	A pointer to a message in which the received data is stored.
	 */
	virtual ReturnValue_t receiveMessage(MessageQueueMessage* message) = 0;
	/**
	 * Deletes all pending messages in the queue.
	 * @param count The number of flushed messages.
	 * @return RETURN_OK on success.
	 */
	virtual ReturnValue_t flush(uint32_t* count) = 0;
	/**
	 * @brief	This method returns the message queue id of the last communication partner.
	 */
	virtual MessageQueueId_t getLastPartner() const = 0;
	/**
	 * @brief	This method returns the message queue id of this class's message queue.
	 */
	virtual MessageQueueId_t getId() const = 0;

	/**
	 * \brief	With the sendMessage call, a queue message is sent to a receiving queue.
	 * \details	This method takes the message provided, adds the sentFrom information and passes
	 * 			it on to the destination provided with an operating system call. The OS's return
	 * 			value is returned.
	 * \param sendTo	This parameter specifies the message queue id to send the message to.
	 * \param message	This is a pointer to a previously created message, which is sent.
	 * \param sentFrom	The sentFrom information can be set to inject the sender's queue id into the message.
	 * 					This variable is set to zero by default.
	 * \param ignoreFault If set to true, the internal software fault counter is not incremented if queue is full (if implemented).
	 */
	virtual ReturnValue_t sendMessageFrom( MessageQueueId_t sendTo, MessageQueueMessage* message, MessageQueueId_t sentFrom, bool ignoreFault = false ) = 0;
	/**
		 * @brief	This operation sends a message to the given destination.
		 * @details	It directly uses the sendMessage call of the MessageQueueSender parent, but passes its
		 * 			queue id as "sentFrom" parameter.
		 * @param sendTo	This parameter specifies the message queue id of the destination message queue.
		 * @param message	A pointer to a previously created message, which is sent.
		 * @param ignoreFault If set to true, the internal software fault counter is not incremented if queue is full.
		 */
	virtual ReturnValue_t sendMessage( MessageQueueId_t sendTo, MessageQueueMessage* message, bool ignoreFault = false ) = 0;

	/**
	 * \brief	The sendToDefaultFrom method sends a queue message to the default destination.
	 * \details	In all other aspects, it works identical to the sendMessage method.
	 * \param message	This is a pointer to a previously created message, which is sent.
	 * \param sentFrom	The sentFrom information can be set to inject the sender's queue id into the message.
	 * 					This variable is set to zero by default.
	 */
	virtual ReturnValue_t sendToDefaultFrom( MessageQueueMessage* message, MessageQueueId_t sentFrom, bool ignoreFault = false ) = 0;
	/**
	 * @brief	This operation sends a message to the default destination.
	 * @details	As in the sendMessage method, this function uses the sendToDefault call of the
	 * 			Implementation class and adds its queue id as "sentFrom" information.
	 * @param message	A pointer to a previously created message, which is sent.
	 */
	virtual ReturnValue_t sendToDefault( MessageQueueMessage* message )  = 0;
	/**
	 * \brief	This method is a simple setter for the default destination.
	 */
	virtual void setDefaultDestination(MessageQueueId_t defaultDestination) = 0;
	/**
	 * \brief	This method is a simple getter for the default destination.
	 */
	virtual MessageQueueId_t getDefaultDestination() const = 0;

	virtual bool isDefaultDestinationSet() const = 0;
};



#endif /* FRAMEWORK_IPC_MESSAGEQUEUEIF_H_ */
