#include <framework/ipc/QueueFactory.h>
#include <mqueue.h>
#include <errno.h>
#include <framework/osal/linux/MessageQueue.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <cstring>

QueueFactory* QueueFactory::factoryInstance = NULL;


ReturnValue_t MessageQueueSenderIF::sendMessage(MessageQueueId_t sendTo,
			MessageQueueMessage* message, MessageQueueId_t sentFrom) {
	message->setSender(sentFrom);
	int result = mq_send(sendTo,
			reinterpret_cast<const char*>(message->getBuffer()), message->messageSize,0);

	//TODO: Check if we're in ISR.
	if (result != 0) {
		//TODO Translate error
		switch(errno){
		case EAGAIN:
			//The O_NONBLOCK flag was set when opening the queue, or the MQ_NONBLOCK flag was set in its attributes, and the specified queue is full.
			return MessageQueueIF::FULL;
		case EBADF:
			//mq_des doesn't represent a valid message queue descriptor, or mq_des wasn't opened for writing.
			error << "MessageQueueSenderIF::sendMessage: Configuration error " << strerror(errno) << " in mq_send mqSendTo: " << sendTo << " sent from " << sentFrom << std::endl;
			/*NO BREAK*/
		case EINTR:
			//The call was interrupted by a signal.
		case EINVAL:
			/*
			 * This value indicates one of the following:
			 * * msg_ptr is NULL.
			 * * msg_len is negative.
			 * * msg_prio is greater than MQ_PRIO_MAX.
			 * * msg_prio is less than 0.
			 * * MQ_PRIO_RESTRICT is set in the mq_attr of mq_des, and msg_prio is greater than the priority of the calling process.
			 * */
		case EMSGSIZE:
			//The msg_len is greater than the msgsize associated with the specified queue.
		default:
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}

QueueFactory* QueueFactory::instance() {
	if (factoryInstance == NULL) {
		factoryInstance = new QueueFactory;
	}
	return factoryInstance;
}

QueueFactory::QueueFactory() {
}

QueueFactory::~QueueFactory() {
}

MessageQueueIF* QueueFactory::createMessageQueue(uint32_t message_depth,
		uint32_t max_message_size) {
	return new MessageQueue(message_depth, max_message_size);
}

void QueueFactory::deleteMessageQueue(MessageQueueIF* queue) {
	delete queue;
}
