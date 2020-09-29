#include "MessageQueue.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../objectmanager/ObjectManagerIF.h"

#include <fstream>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <cstring>
#include <errno.h>



MessageQueue::MessageQueue(uint32_t messageDepth, size_t maxMessageSize):
		id(MessageQueueIF::NO_QUEUE),lastPartner(MessageQueueIF::NO_QUEUE),
		defaultDestination(MessageQueueIF::NO_QUEUE),
		maxMessageSize(maxMessageSize) {
	//debug << "MessageQueue::MessageQueue: Creating a queue" << std::endl;
	mq_attr attributes;
	this->id = 0;
	//Set attributes
	attributes.mq_curmsgs = 0;
	attributes.mq_maxmsg = messageDepth;
	attributes.mq_msgsize = maxMessageSize;
	attributes.mq_flags = 0; //Flags are ignored on Linux during mq_open
	//Set the name of the queue. The slash is mandatory!
	sprintf(name, "/FSFW_MQ%u\n", queueCounter++);

	// Create a nonblocking queue if the name is available (the queue is read
	// and writable for the owner as well as the group)
	int oflag = O_NONBLOCK | O_RDWR | O_CREAT | O_EXCL;
	mode_t mode = S_IWUSR | S_IREAD | S_IWGRP | S_IRGRP | S_IROTH | S_IWOTH;
	mqd_t tempId = mq_open(name, oflag, mode, &attributes);
	if (tempId == -1) {
		handleError(&attributes, messageDepth);
	}
	else {
		//Successful mq_open call
		this->id = tempId;
	}
}

MessageQueue::~MessageQueue() {
	int status = mq_close(this->id);
	if(status != 0){
		sif::error << "MessageQueue::Destructor: mq_close Failed with status: "
				   << strerror(errno) <<std::endl;
	}
	status = mq_unlink(name);
	if(status != 0){
		sif::error << "MessageQueue::Destructor: mq_unlink Failed with status: "
				   << strerror(errno) << std::endl;
	}
}

ReturnValue_t MessageQueue::handleError(mq_attr* attributes,
		uint32_t messageDepth) {
	switch(errno) {
	case(EINVAL): {
		sif::error << "MessageQueue::MessageQueue: Invalid name or attributes"
				" for message size" << std::endl;
		size_t defaultMqMaxMsg = 0;
		// Not POSIX conformant, but should work for all UNIX systems.
		// Just an additional helpful printout :-)
		if(std::ifstream("/proc/sys/fs/mqueue/msg_max",std::ios::in) >>
				defaultMqMaxMsg and defaultMqMaxMsg < messageDepth) {
			/*
			See: https://www.man7.org/linux/man-pages/man3/mq_open.3.html
			This happens if the msg_max value is not large enough
			It is ignored if the executable is run in privileged mode.
		    Run the unlockRealtime script or grant the mode manually by using:
			sudo setcap 'CAP_SYS_RESOURCE=+ep' <pathToBinary>

			Persistent solution for session:
			echo <newMsgMax> | sudo tee /proc/sys/fs/mqueue/msg_max

			Permanent solution:
			sudo nano /etc/sysctl.conf
			Append at end: fs/mqueue/msg_max = <newMsgMaxLen>
			Apply changes with: sudo sysctl -p
			*/
			sif::error << "MessageQueue::MessageQueue: Default MQ size "
					<< defaultMqMaxMsg << " is too small for requested size "
					<< messageDepth << std::endl;
			sif::error << "This error can be fixed by setting the maximum "
					"allowed message size higher!" << std::endl;

		}
		break;
	}
	case(EEXIST): {
		// An error occured during open
		// We need to distinguish if it is caused by an already created queue
		//There's another queue with the same name
		//We unlink the other queue
		int status = mq_unlink(name);
		if (status != 0) {
			sif::error << "mq_unlink Failed with status: " << strerror(errno)
													<< std::endl;
		}
		else {
			// Successful unlinking, try to open again
			mqd_t tempId = mq_open(name,
					O_NONBLOCK | O_RDWR | O_CREAT | O_EXCL,
					S_IWUSR | S_IREAD | S_IWGRP | S_IRGRP, attributes);
			if (tempId != -1) {
				//Successful mq_open
				this->id = tempId;
				return HasReturnvaluesIF::RETURN_OK;
			}
		}
		break;
	}

	default:
		// Failed either the first time or the second time
		sif::error << "MessageQueue::MessageQueue: Creating Queue " << std::hex
		<< name << std::dec << " failed with status: "
		<< strerror(errno) << std::endl;

	}
	return HasReturnvaluesIF::RETURN_FAILED;



}

ReturnValue_t MessageQueue::sendMessage(MessageQueueId_t sendTo,
		MessageQueueMessageIF* message, bool ignoreFault) {
	return sendMessageFrom(sendTo, message, this->getId(), false);
}

ReturnValue_t MessageQueue::sendToDefault(MessageQueueMessageIF* message) {
	return sendToDefaultFrom(message, this->getId());
}

ReturnValue_t MessageQueue::reply(MessageQueueMessageIF* message) {
	if (this->lastPartner != 0) {
		return sendMessageFrom(this->lastPartner, message, this->getId());
	} else {
		return NO_REPLY_PARTNER;
	}
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessageIF* message,
		MessageQueueId_t* receivedFrom) {
	ReturnValue_t status = this->receiveMessage(message);
	*receivedFrom = this->lastPartner;
	return status;
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessageIF* message) {
	if(message == nullptr) {
		sif::error << "MessageQueue::receiveMessage: Message is "
				"nullptr!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	if(message->getMaximumMessageSize() < maxMessageSize) {
		sif::error << "MessageQueue::receiveMessage: Message size "
				<< message->getMaximumMessageSize()
				<< " too small to receive data!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	unsigned int messagePriority = 0;
	int status = mq_receive(id,reinterpret_cast<char*>(message->getBuffer()),
			message->getMaximumMessageSize(),&messagePriority);
	if (status > 0) {
		this->lastPartner = message->getSender();
		//Check size of incoming message.
		if (message->getMessageSize() < message->getMinimumMessageSize()) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}else if(status==0){
		//Success but no message received
		return MessageQueueIF::EMPTY;
	} else {
		//No message was received. Keep lastPartner anyway, I might send
		//something later. But still, delete packet content.
		memset(message->getData(), 0, message->getMaximumMessageSize());
		switch(errno){
		case EAGAIN:
			//O_NONBLOCK or MQ_NONBLOCK was set and there are no messages
			//currently on the specified queue.
			return MessageQueueIF::EMPTY;
		case EBADF:
			//mqdes doesn't represent a valid queue open for reading.
			sif::error << "MessageQueue::receive: configuration error "
			           << strerror(errno)  << std::endl;
			/*NO BREAK*/
		case EINVAL:
			/*
			 * This value indicates one of the following:
			 * - The pointer to the buffer for storing the received message,
			 *   msg_ptr, is NULL.
			 * - The number of bytes requested, msg_len is less than zero.
			 * - msg_len is anything other than the mq_msgsize of the specified
			 *   queue, and the QNX extended option MQ_READBUF_DYNAMIC hasn't
			 *   been set in the queue's mq_flags.
			 */
			sif::error << "MessageQueue::receive: configuration error "
					   << strerror(errno)  << std::endl;
			/*NO BREAK*/
		case EMSGSIZE:
			/*
			 * This value indicates one of the following:
			 * - the QNX extended option MQ_READBUF_DYNAMIC hasn't been set,
			 *   and the given msg_len is shorter than the mq_msgsize for
			 *   the given queue.
			 * - the extended option MQ_READBUF_DYNAMIC has been set, but the
			 *   given msg_len is too short for the message that would have
			 *   been received.
			 */
			sif::error << "MessageQueue::receive: configuration error "
			           << strerror(errno)  << std::endl;
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
			sif::error << "MessageQueue::flush configuration error, "
					"called flush with an invalid queue ID" << std::endl;
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
			sif::error << "MessageQueue::flush configuration error, "
					"called flush with an invalid queue ID" << std::endl;
			/*NO BREAK*/
		case EINVAL:
			/*
			 * This value indicates one of the following:
			 *  - mq_attr is NULL.
			 *  - MQ_MULT_NOTIFY had been set for this queue, and the given
			 *    mq_flags includes a 0 in the MQ_MULT_NOTIFY bit. Once
			 *    MQ_MULT_NOTIFY has been turned on, it may never be turned off.
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

ReturnValue_t MessageQueue::sendToDefaultFrom(MessageQueueMessageIF* message,
		MessageQueueId_t sentFrom, bool ignoreFault) {
	return sendMessageFrom(defaultDestination, message, sentFrom, ignoreFault);
}


ReturnValue_t MessageQueue::sendMessageFrom(MessageQueueId_t sendTo,
		MessageQueueMessageIF* message, MessageQueueId_t sentFrom,
		bool ignoreFault) {
	return sendMessageFromMessageQueue(sendTo,message, sentFrom,ignoreFault);

}

MessageQueueId_t MessageQueue::getDefaultDestination() const {
	return this->defaultDestination;
}

bool MessageQueue::isDefaultDestinationSet() const {
	return (defaultDestination != NO_QUEUE);
}

uint16_t MessageQueue::queueCounter = 0;

ReturnValue_t MessageQueue::sendMessageFromMessageQueue(MessageQueueId_t sendTo,
		MessageQueueMessageIF *message, MessageQueueId_t sentFrom,
		bool ignoreFault) {
	if(message == nullptr) {
		sif::error << "MessageQueue::sendMessageFromMessageQueue: Message is "
				"nullptr!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	message->setSender(sentFrom);
	int result = mq_send(sendTo,
			reinterpret_cast<const char*>(message->getBuffer()),
			message->getMessageSize(),0);

	//TODO: Check if we're in ISR.
	if (result != 0) {
		if(!ignoreFault){
			InternalErrorReporterIF* internalErrorReporter =
					objectManager->get<InternalErrorReporterIF>(
						objects::INTERNAL_ERROR_REPORTER);
			if (internalErrorReporter != NULL) {
				internalErrorReporter->queueMessageNotSent();
			}
		}
		switch(errno){
		case EAGAIN:
			//The O_NONBLOCK flag was set when opening the queue, or the
			//MQ_NONBLOCK flag was set in its attributes, and the
			//specified queue is full.
			return MessageQueueIF::FULL;
		case EBADF: {
			//mq_des doesn't represent a valid message queue descriptor,
			//or mq_des wasn't opened for writing.
			sif::error << "MessageQueue::sendMessage: Configuration error, MQ"
					<< " destination invalid."  << std::endl;
			sif::error << strerror(errno) << " in "
					<<"mq_send to: " << sendTo << " sent from "
					<< sentFrom << std::endl;
			return DESTINVATION_INVALID;
		}
		case EINTR:
			//The call was interrupted by a signal.
		case EINVAL:
			/*
			 * This value indicates one of the following:
			 * - msg_ptr is NULL.
			 * - msg_len is negative.
			 * - msg_prio is greater than MQ_PRIO_MAX.
			 * - msg_prio is less than 0.
			 * - MQ_PRIO_RESTRICT is set in the mq_attr of mq_des, and
			 *   msg_prio is greater than the priority of the calling process.
			 */
			sif::error << "MessageQueue::sendMessage: Configuration error "
			           << strerror(errno) << " in mq_send" << std::endl;
			/*NO BREAK*/
		case EMSGSIZE:
			// The msg_len is greater than the msgsize associated with
			//the specified queue.
			sif::error << "MessageQueue::sendMessage: Size error [" <<
					strerror(errno) << "] in mq_send" << std::endl;
			/*NO BREAK*/
		default:
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}
