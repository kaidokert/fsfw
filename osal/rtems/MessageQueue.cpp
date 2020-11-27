#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../objectmanager/ObjectManagerIF.h"
#include "MessageQueue.h"
#include "RtemsBasic.h"
#include <cstring>
MessageQueue::MessageQueue(size_t message_depth, size_t max_message_size) :
		id(0), lastPartner(0), defaultDestination(NO_QUEUE), internalErrorReporter(nullptr) {
	rtems_name name = ('Q' << 24) + (queueCounter++ << 8);
	rtems_status_code status = rtems_message_queue_create(name, message_depth,
			max_message_size, 0, &(this->id));
	if (status != RTEMS_SUCCESSFUL) {
		sif::error << "MessageQueue::MessageQueue: Creating Queue " << std::hex
				<< name << std::dec << " failed with status:"
				<< (uint32_t) status << std::endl;
		this->id = 0;
	}
}

MessageQueue::~MessageQueue() {
	rtems_message_queue_delete(id);
}

ReturnValue_t MessageQueue::sendMessage(MessageQueueId_t sendTo,
		MessageQueueMessageIF* message, bool ignoreFault) {
	return sendMessageFrom(sendTo, message, this->getId(), ignoreFault);
}

ReturnValue_t MessageQueue::sendToDefault(MessageQueueMessageIF* message) {
	return sendToDefaultFrom(message, this->getId());
}

ReturnValue_t MessageQueue::reply(MessageQueueMessageIF* message) {
	if (this->lastPartner != 0) {
		return sendMessage(this->lastPartner, message, this->getId());
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
	size_t size = 0;
	rtems_status_code status = rtems_message_queue_receive(id,
			message->getBuffer(),&size,
			RTEMS_NO_WAIT, 1);
	if (status == RTEMS_SUCCESSFUL) {
		message->setMessageSize(size);
		this->lastPartner = message->getSender();
		//Check size of incoming message.
		if (message->getMessageSize() < message->getMinimumMessageSize()) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	} else {
		//No message was received. Keep lastPartner anyway, I might send something later.
		//But still, delete packet content.
		memset(message->getData(), 0, message->getMaximumMessageSize());
	}
	return convertReturnCode(status);
}

MessageQueueId_t MessageQueue::getLastPartner() const {
	return this->lastPartner;
}

ReturnValue_t MessageQueue::flush(uint32_t* count) {
	rtems_status_code status = rtems_message_queue_flush(id, count);
	return convertReturnCode(status);
}

MessageQueueId_t MessageQueue::getId() const {
	return this->id;
}

void MessageQueue::setDefaultDestination(MessageQueueId_t defaultDestination) {
	this->defaultDestination = defaultDestination;
}

ReturnValue_t MessageQueue::sendMessageFrom(MessageQueueId_t sendTo,
		MessageQueueMessageIF* message, MessageQueueId_t sentFrom,
		bool ignoreFault) {

	message->setSender(sentFrom);
	rtems_status_code result = rtems_message_queue_send(sendTo,
			message->getBuffer(), message->getMessageSize());

	//TODO: Check if we're in ISR.
	if (result != RTEMS_SUCCESSFUL && !ignoreFault) {
		if (internalErrorReporter == nullptr) {
			internalErrorReporter = objectManager->get<InternalErrorReporterIF>(
					objects::INTERNAL_ERROR_REPORTER);
		}
		if (internalErrorReporter != nullptr) {
			internalErrorReporter->queueMessageNotSent();
		}
	}

	ReturnValue_t returnCode = convertReturnCode(result);
	if(result == MessageQueueIF::EMPTY){
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	return returnCode;
}

ReturnValue_t MessageQueue::sendToDefaultFrom(MessageQueueMessageIF* message,
		MessageQueueId_t sentFrom, bool ignoreFault) {
	return sendMessageFrom(defaultDestination, message, sentFrom, ignoreFault);
}

MessageQueueId_t MessageQueue::getDefaultDestination() const {
	return this->defaultDestination;
}

bool MessageQueue::isDefaultDestinationSet() const {
	return (defaultDestination != NO_QUEUE);
}

ReturnValue_t MessageQueue::convertReturnCode(rtems_status_code inValue){
		switch(inValue){
		case RTEMS_SUCCESSFUL:
			return HasReturnvaluesIF::RETURN_OK;
		case  RTEMS_INVALID_ID:
			return HasReturnvaluesIF::RETURN_FAILED;
		case RTEMS_TIMEOUT:
			return HasReturnvaluesIF::RETURN_FAILED;
		case RTEMS_OBJECT_WAS_DELETED:
			return HasReturnvaluesIF::RETURN_FAILED;
		case RTEMS_INVALID_ADDRESS:
			return HasReturnvaluesIF::RETURN_FAILED;
		case RTEMS_INVALID_SIZE:
			return HasReturnvaluesIF::RETURN_FAILED;
		case RTEMS_TOO_MANY:
			return MessageQueueIF::FULL;
		case RTEMS_UNSATISFIED:
			return MessageQueueIF::EMPTY;
		default:
			return HasReturnvaluesIF::RETURN_FAILED;
		}

}



uint16_t MessageQueue::queueCounter = 0;
