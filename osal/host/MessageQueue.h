#ifndef FRAMEWORK_OSAL_HOST_MESSAGEQUEUE_H_
#define FRAMEWORK_OSAL_HOST_MESSAGEQUEUE_H_

#include "../../internalError/InternalErrorReporterIF.h"
#include "../../ipc/MessageQueueIF.h"
#include "../../ipc/MessageQueueMessage.h"
#include "../../ipc/MutexIF.h"
#include "../../timemanager/Clock.h"

#include <queue>
#include <memory>

/**
 * @brief		This class manages sending and receiving of
 * 				message queue messages.
 * @details
 * Message queues are used to pass asynchronous messages between processes.
 * They work like post boxes, where all incoming messages are stored in FIFO
 * order. This class creates a new receiving queue and provides methods to fetch
 * received messages. Being a child of MessageQueueSender, this class also
 * provides methods to send a message to a user-defined or a default destination.
 * In addition it also provides a reply method to answer to the queue it
 * received its last message from.
 *
 * The MessageQueue should be used as "post box" for a single owning object.
 * So all message queue communication is "n-to-one".
 * For creating the queue, as well as sending and receiving messages, the class
 * makes use of the operating system calls provided.
 *
 * Please keep in mind that FreeRTOS offers different calls for message queue
 * operations if called from an ISR.
 * For now, the system context needs to be switched manually.
 * @ingroup osal
 * @ingroup message_queue
 */
class MessageQueue : public MessageQueueIF {
	friend class MessageQueueSenderIF;
public:
	/**
	 * @brief	The constructor initializes and configures the message queue.
	 * @details
	 * By making use of the according operating system call, a message queue is
	 * created and initialized. The message depth - the maximum number of
	 * messages to be buffered - may be set with the help of a parameter,
	 * whereas the message size is automatically set to the maximum message
	 * queue message size. The operating system sets the message queue id, or
	 * in case of failure, it is set to zero.
	 * @param message_depth
	 * The number of messages to be buffered before passing an error to the
	 * sender. Default is three.
	 * @param max_message_size
	 * With this parameter, the maximum message size can be adjusted.
	 * This should be left default.
	 */
	MessageQueue(size_t messageDepth = 3,
			size_t maxMessageSize = MessageQueueMessage::MAX_MESSAGE_SIZE);

	/** Copying message queues forbidden */
	MessageQueue(const MessageQueue&) = delete;
	MessageQueue& operator=(const MessageQueue&) = delete;

	/**
	 * @brief	The destructor deletes the formerly created message queue.
	 * @details	This is accomplished by using the delete call provided
	 * 			by the operating system.
	 */
	virtual ~MessageQueue();

	/**
	 * @brief	This operation sends a message to the given destination.
	 * @details	It directly uses the sendMessage call of the MessageQueueSender
	 * parent, but passes its queue id as "sentFrom" parameter.
	 * @param sendTo	This parameter specifies the message queue id of the
	 *  destination message queue.
	 * @param message	A pointer to a previously created message, which is sent.
	 * @param ignoreFault If set to true, the internal software fault counter
	 * is not incremented if queue is full.
	 */
	ReturnValue_t sendMessage(MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, bool ignoreFault = false) override;
	/**
	 * @brief	This operation sends a message to the default destination.
	 * @details	As in the sendMessage method, this function uses the
	 * sendToDefault call of the MessageQueueSender parent class and adds its
	 * queue id as "sentFrom" information.
	 * @param message	A pointer to a previously created message, which is sent.
	 */
	ReturnValue_t sendToDefault(MessageQueueMessageIF* message) override;
	/**
	 * @brief	This operation sends a message to the last communication partner.
	 * @details	This operation simplifies answering an incoming message by using
	 * the stored lastPartner information as destination. If there was no
	 * message received yet (i.e. lastPartner is zero), an error code is returned.
	 * @param message	A pointer to a previously created message, which is sent.
	 */
	ReturnValue_t reply(MessageQueueMessageIF* message) override;

	/**
	 * @brief	With the sendMessage call, a queue message is sent to a
	 * 			receiving queue.
	 * @details
	 * This method takes the message provided, adds the sentFrom information and
	 * passes it on to the destination provided with an operating system call.
	 *  The OS's return value is returned.
	 * @param sendTo	This parameter specifies the message queue id to send
	 * the message to.
	 * @param message	This is a pointer to a previously created message,
	 * which is sent.
	 * @param sentFrom	The sentFrom information can be set to inject the
	 * sender's queue id into the message. This variable is set to zero by
	 * default.
	 * @param ignoreFault If set to true, the internal software fault counter
	 * is not incremented if queue is full.
	 */
	virtual ReturnValue_t sendMessageFrom( MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, MessageQueueId_t sentFrom = NO_QUEUE,
			bool ignoreFault = false) override;

	/**
	 * @brief	The sendToDefault method sends a queue message to the default
	 * 			destination.
	 * @details
	 * In all other aspects, it works identical to the sendMessage method.
	 * @param message	This is a pointer to a previously created message,
	 * which is sent.
	 * @param sentFrom	The sentFrom information can be set to inject the
	 * sender's queue id into the message. This variable is set to zero by
	 * default.
	 */
	virtual ReturnValue_t sendToDefaultFrom( MessageQueueMessageIF* message,
			MessageQueueId_t sentFrom = NO_QUEUE,
			bool ignoreFault = false) override;

	/**
	 * @brief	This function reads available messages from the message queue
	 * 			and returns the sender.
	 * @details
	 * It works identically to the other receiveMessage call, but in  addition
	 * returns the sender's queue id.
	 * @param message	A pointer to a message in which the received data is stored.
	 * @param receivedFrom	A pointer to a queue id in which the sender's id is stored.
	 */
	ReturnValue_t receiveMessage(MessageQueueMessageIF* message,
			MessageQueueId_t *receivedFrom) override;

	/**
	 * @brief	This function reads available messages from the message queue.
	 * @details
	 * If data is available it is stored in the passed message pointer.
	 * The message's original content is overwritten and the sendFrom
	 * information is stored in the lastPartner attribute. Else, the lastPartner
	 * information remains untouched, the message's content is cleared and the
	 * function returns immediately.
	 * @param message	A pointer to a message in which the received data is stored.
	 */
	ReturnValue_t receiveMessage(MessageQueueMessageIF* message) override;
	/**
	 * Deletes all pending messages in the queue.
	 * @param count The number of flushed messages.
	 * @return RETURN_OK on success.
	 */
	ReturnValue_t flush(uint32_t* count) override;
	/**
	 * @brief	This method returns the message queue id of the last
	 * 			communication partner.
	 */
	MessageQueueId_t getLastPartner() const override;
	/**
	 * @brief	This method returns the message queue id of this class's
	 * 			message queue.
	 */
	MessageQueueId_t getId() const override;

	/**
	 * @brief	This method is a simple setter for the default destination.
	 */
	void setDefaultDestination(MessageQueueId_t defaultDestination) override;
	/**
	 * @brief	This method is a simple getter for the default destination.
	 */
	MessageQueueId_t getDefaultDestination() const override;

	bool isDefaultDestinationSet() const override;

	ReturnValue_t lockQueue(MutexIF::TimeoutType timeoutType,
	        dur_millis_t lockTimeout);
	ReturnValue_t unlockQueue();
protected:
	/**
	 * @brief 	Implementation to be called from any send Call within
	 * 			MessageQueue and MessageQueueSenderIF.
	 * @details
	 * This method takes the message provided, adds the sentFrom information and
	 * passes it on to the destination provided with an operating system call.
	 * The OS's return value is returned.
	 * @param sendTo
	 * This parameter specifies the message queue id to send the message to.
	 * @param message
	 * This is a pointer to a previously created message, which is sent.
	 * @param sentFrom
	 * The sentFrom information can be set to inject the sender's queue id into
	 * the message. This variable is set to zero by default.
	 * @param ignoreFault
	 * If set to true, the internal software fault counter  is not incremented
	 * if queue is full.
	 * @param context Specify whether call is made from task or from an ISR.
	 */
	static ReturnValue_t sendMessageFromMessageQueue(MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, MessageQueueId_t sentFrom = NO_QUEUE,
			bool ignoreFault=false);

	//static ReturnValue_t handleSendResult(BaseType_t result, bool ignoreFault);

private:
	std::queue<MessageQueueMessage> messageQueue;
	/**
	 * @brief	The class stores the queue id it got assigned.
	 * 			If initialization fails, the queue id is set to zero.
	 */
	MessageQueueId_t mqId = 0;
	size_t messageSize = 0;
	size_t messageDepth = 0;

	MutexIF* queueLock;

	bool defaultDestinationSet = false;
	MessageQueueId_t defaultDestination = 0;
	MessageQueueId_t lastPartner = 0;
};

#endif /* FRAMEWORK_OSAL_HOST_MESSAGEQUEUE_H_ */
