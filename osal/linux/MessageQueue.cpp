#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <cstring>
#include <errno.h>
#include <framework/osal/linux/MessageQueue.h>


MessageQueue::MessageQueue(size_t message_depth, size_t max_message_size) :
		id(0), lastPartner(0), defaultDestination(NO_QUEUE) {
	//debug << "MessageQueue::MessageQueue: Creating a queue" << std::endl;
	mq_attr attributes;
	this->id = 0;
	//Set attributes
	attributes.mq_curmsgs = 0;
	attributes.mq_maxmsg = message_depth;
	attributes.mq_msgsize = max_message_size;
	attributes.mq_flags = 0; //Flags are ignored on Linux during mq_open

	//Set the name of the queue
	sprintf(name, "/Q%u\n", queueCounter++);

	//Create a nonblocking queue if the name is available (the queue is Read and writable for the owner as well as the group)
	mqd_t tempId = mq_open(name, O_NONBLOCK | O_RDWR | O_CREAT | O_EXCL,
	S_IWUSR | S_IREAD | S_IWGRP | S_IRGRP | S_IROTH | S_IWOTH, &attributes);
	if (tempId == -1) {
		//An error occured during open
		//We need to distinguish if it is caused by an already created queue
		if (errno == EEXIST) {
			//There's another queue with the same name
			//We unlink the other queue
			int status = mq_unlink(name);
			if (status != 0) {
				error << "mq_unlink Failed with status: " << strerror(errno)
						<< std::endl;
			} else {
				//Successful unlinking, try to open again
				mqd_t tempId = mq_open(name,
				O_NONBLOCK | O_RDWR | O_CREAT | O_EXCL,
				S_IWUSR | S_IREAD | S_IWGRP | S_IRGRP, &attributes);
				if (tempId != -1) {
					//Successful mq_open
					this->id = tempId;
					return;
				}
			}
		}
		//Failed either the first time or the second time
		error << "MessageQueue::MessageQueue: Creating Queue " << std::hex
				<< name << std::dec << " failed with status: "
				<< strerror(errno) << std::endl;
	} else {
		//Successful mq_open call
		this->id = tempId;
	}
}

MessageQueue::~MessageQueue() {
	int status = mq_close(this->id);
	if(status != 0){
		error << "MessageQueue::Destructor: mq_close Failed with status: " << strerror(errno) <<std::endl;
	}
	status = mq_unlink(name);
	if(status != 0){
		error << "MessageQueue::Destructor: mq_unlink Failed with status: " << strerror(errno) <<std::endl;
	}
}

ReturnValue_t MessageQueue::sendMessage(MessageQueueId_t sendTo,
		MessageQueueMessage* message, bool ignoreFault) {
	return sendMessageFrom(sendTo, message, this->getId(), false);
}

ReturnValue_t MessageQueue::sendToDefault(MessageQueueMessage* message) {
	return sendToDefaultFrom(message, this->getId());
}

ReturnValue_t MessageQueue::reply(MessageQueueMessage* message) {
	if (this->lastPartner != 0) {
		return sendMessageFrom(this->lastPartner, message, this->getId());
	} else {
		return NO_REPLY_PARTNER;
	}
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessage* message,
		MessageQueueId_t* receivedFrom) {
	ReturnValue_t status = this->receiveMessage(message);
	*receivedFrom = this->lastPartner;
	return status;
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessage* message) {
	unsigned int messagePriority = 0;
	int status = mq_receive(id,reinterpret_cast<char*>(message->getBuffer()),message->MAX_MESSAGE_SIZE,&messagePriority);
	if (status > 0) {
		this->lastPartner = message->getSender();
		//Check size of incoming message.
		if (message->messageSize < message->getMinimumMessageSize()) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}else if(status==0){
		//Success but no message received
		return MessageQueueIF::EMPTY;
	} else {
		//No message was received. Keep lastPartner anyway, I might send something later.
		//But still, delete packet content.
		memset(message->getData(), 0, message->MAX_DATA_SIZE);
		switch(errno){
		case EAGAIN:
			//O_NONBLOCK or MQ_NONBLOCK was set and there are no messages currently on the specified queue.
			return MessageQueueIF::EMPTY;
		case EBADF:
			//mqdes doesn't represent a valid queue open for reading.
			error << "MessageQueue::receive: configuration error " << strerror(errno)  << std::endl;
			/*NO BREAK*/
		case EINVAL:
			/*
			 * This value indicates one of the following:
			 * * The pointer to the buffer for storing the received message, msg_ptr, is NULL.
			 * * The number of bytes requested, msg_len is less than zero.
			 * * msg_len is anything other than the mq_msgsize of the specified queue, and the QNX extended option MQ_READBUF_DYNAMIC hasn't been set in the queue's mq_flags.
			 */
			error << "MessageQueue::receive: configuration error " << strerror(errno)  << std::endl;
			/*NO BREAK*/
		case EMSGSIZE:
			/*
			 * This value indicates one of the following:
			 * * the QNX extended option MQ_READBUF_DYNAMIC hasn't been set, and the given msg_len is shorter than the mq_msgsize for the given queue.
			 * * the extended option MQ_READBUF_DYNAMIC has been set, but the given msg_len is too short for the message that would have been received.
			 */
			error << "MessageQueue::receive: configuration error " << strerror(errno)  << std::endl;
			/*NO BREAK*/
		case EINTR:
			//The operation was interrupted by a signal.
		default:

			return HasReturnvaluesIF::RETURN_FAILED;
		}

	}
}

MessageQueueId_t MessageQueue::getLastPartner() const {
	return this->lastPartner;
}

ReturnValue_t MessageQueue::flush(uint32_t* count) {
	mq_attr attrib;
	int status = mq_getattr(id,&attrib);
	if(status != 0){
		switch(errno){
		case EBADF:
			//mqdes doesn't represent a valid message queue.
			error << "MessageQueue::flush configuration error, called flush with an invalid queue ID" << std::endl;
			/*NO BREAK*/
		case EINVAL:
			//mq_attr is NULL
		default:
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	*count = attrib.mq_curmsgs;
	attrib.mq_curmsgs = 0;
	status = mq_setattr(id,&attrib,NULL);
	if(status != 0){
		switch(errno){
		case EBADF:
			//mqdes doesn't represent a valid message queue.
			error << "MessageQueue::flush configuration error, called flush with an invalid queue ID" << std::endl;
			/*NO BREAK*/
		case EINVAL:
			/*
			 * This value indicates one of the following:
			 * * mq_attr is NULL.
			 * * MQ_MULT_NOTIFY had been set for this queue, and the given mq_flags includes a 0 in the MQ_MULT_NOTIFY bit. Once MQ_MULT_NOTIFY has been turned on, it may never be turned off.
			 *
			 */
		default:
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t MessageQueue::getId() const {
	return this->id;
}

void MessageQueue::setDefaultDestination(MessageQueueId_t defaultDestination) {
	this->defaultDestination = defaultDestination;
}

ReturnValue_t MessageQueue::sendMessageFrom(MessageQueueId_t sendTo,
		MessageQueueMessage* message, MessageQueueId_t sentFrom,
		bool ignoreFault) {
	return sendMessageFromMessageQueue(sendTo,message,sentFrom,ignoreFault);

}

ReturnValue_t MessageQueue::sendToDefaultFrom(MessageQueueMessage* message,
		MessageQueueId_t sentFrom, bool ignoreFault) {
	return sendMessageFrom(defaultDestination, message, sentFrom, ignoreFault);
}

MessageQueueId_t MessageQueue::getDefaultDestination() const {
	return this->defaultDestination;
}

bool MessageQueue::isDefaultDestinationSet() const {
	return (defaultDestination != NO_QUEUE);
}

uint16_t MessageQueue::queueCounter = 0;

ReturnValue_t MessageQueue::sendMessageFromMessageQueue(MessageQueueId_t sendTo,
		MessageQueueMessage *message, MessageQueueId_t sentFrom,
		bool ignoreFault) {
	message->setSender(sentFrom);
	int result = mq_send(sendTo,
			reinterpret_cast<const char*>(message->getBuffer()), message->messageSize,0);

	//TODO: Check if we're in ISR.
	if (result != 0) {
		if(!ignoreFault){
			InternalErrorReporterIF* internalErrorReporter = objectManager->get<InternalErrorReporterIF>(
						objects::INTERNAL_ERROR_REPORTER);
			if (internalErrorReporter != NULL) {
				internalErrorReporter->queueMessageNotSent();
			}
		}
		switch(errno){
		case EAGAIN:
			//The O_NONBLOCK flag was set when opening the queue, or the MQ_NONBLOCK flag was set in its attributes, and the specified queue is full.
			return MessageQueueIF::FULL;
		case EBADF:
			//mq_des doesn't represent a valid message queue descriptor, or mq_des wasn't opened for writing.
			error << "MessageQueue::sendMessage: Configuration error " << strerror(errno) << " in mq_send mqSendTo: " << sendTo << " sent from " << sentFrom << std::endl;
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
			error << "MessageQueue::sendMessage: Configuration error " << strerror(errno) << " in mq_send" << std::endl;
			/*NO BREAK*/
		case EMSGSIZE:
			//The msg_len is greater than the msgsize associated with the specified queue.
		default:
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}
