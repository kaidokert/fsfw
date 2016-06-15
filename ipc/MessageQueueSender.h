#ifndef MESSAGEQUEUESENDER_H_
#define MESSAGEQUEUESENDER_H_

#include <framework/events/Event.h>
#include <framework/ipc/MessageQueueMessage.h>
#include <framework/osal/OSAL.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>

/**
 * \defgroup message_queue Message Queues
 * This group contains all Message Queue elements, but also the different message
 * types sent over these queues.
 */
/**
 *	\brief		This class manages sending of messages to receiving message queues.
 *
 *	\details	Message queues are a typical method of interprocess communication.
 *				They work like post boxes, where all incoming messages are stored in FIFO
 *				order. This class provides an interface to simplify sending messages to
 *				receiving queues without the necessity of owing a "post box" itself. It makes
 *				use of the underlying operating system's message queue features.
 *	\ingroup message_queue
 */
class MessageQueueSender : public HasReturnvaluesIF {
private:
	/**
	 * \brief	This attribute stores a default destination to send messages to.
	 * \details	It is stored to simplify sending to always-the-same receiver. The attribute may
	 * 			be set in the constructor or by a setter call to setDefaultDestination.
	 */
	MessageQueueId_t default_destination;
public:

	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::OBSW_1;
	static const Event SEND_MSG_FAILED = MAKE_EVENT(0, SEVERITY::LOW);
	static const MessageQueueId_t NO_QUEUE = 0;
	/**
	 * \brief	In the constructor of the class, the default destination may be set.
	 * \details	As the MessageQueueSender class has no receiving queue by itself, no
	 * 			operating system call to create a message queue is required.
	 * \param	set_default_destination	With this parameter, the default destination is set.
	 * 									If no value is provided, it is set to zero.
	 */
	MessageQueueSender( MessageQueueId_t set_default_destination = NO_QUEUE );
	/**
	 * \brief	As almost nothing is done in the constructor, there's nothing done in the destructor as well.
	 */
	virtual ~MessageQueueSender();
	/**
	 * \brief	With the sendMessage call, a queue message is sent to a receiving queue.
	 * \details	This method takes the message provided, adds the sentFrom information and passes
	 * 			it on to the destination provided with an operating system call. The OS's return
	 * 			value is returned.
	 * \param sendTo	This parameter specifies the message queue id to send the message to.
	 * \param message	This is a pointer to a previously created message, which is sent.
	 * \param sentFrom	The sentFrom information can be set to inject the sender's queue id into the message.
	 * 					This variable is set to zero by default.
	 */
	virtual ReturnValue_t sendMessage( MessageQueueId_t sendTo, MessageQueueMessage* message, MessageQueueId_t sentFrom = NO_QUEUE );
	/**
	 * \brief	The sendToDefault method sends a queue message to the default destination.
	 * \details	In all other aspects, it works identical to the sendMessage method.
	 * \param message	This is a pointer to a previously created message, which is sent.
	 * \param sentFrom	The sentFrom information can be set to inject the sender's queue id into the message.
	 * 					This variable is set to zero by default.
	 */
	virtual ReturnValue_t sendToDefault( MessageQueueMessage* message, MessageQueueId_t sentFrom = NO_QUEUE );
	/**
	 * \brief	This method is a simple setter for the default destination.
	 */
	void setDefaultDestination(MessageQueueId_t defaultDestination);
	/**
	 * \brief	This method is a simple getter for the default destination.
	 */
	MessageQueueId_t getDefaultDestination();
};


#endif /* MESSAGEQUEUESENDER_H_ */
