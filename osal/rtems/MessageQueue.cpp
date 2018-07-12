#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include "MessageQueue.h"
#include "RtemsBasic.h"

MessageQueue::MessageQueue(size_t message_depth, size_t max_message_size) :
		id(0), lastPartner(0), defaultDestination(NO_QUEUE), internalErrorReporter(NULL) {
	rtems_name name = ('Q' << 24) + (queueCounter++ << 8);
	rtems_status_code status = rtems_message_queue_create(name, message_depth,
			max_message_size, 0, &(this->id));
	if (status != RTEMS_SUCCESSFUL) {
		error << "MessageQueue::MessageQueue: Creating Queue " << std::hex
				<< name << std::dec << " failed with status:"
				<< (uint32_t) status << std::endl;
		this->id = 0;
	}
}

MessageQueue::~MessageQueue() {
	rtems_message_queue_delete(id);
}

ReturnValue_t MessageQueue::sendMessage(MessageQueueId_t sendTo,
		MessageQueueMessage* message, bool ignoreFault) {
	return sendMessage(sendTo, message, this->getId(), ignoreFault);
}

ReturnValue_t MessageQueue::sendToDefault(MessageQueueMessage* message) {
	return sendToDefault(message, this->getId());
}

ReturnValue_t MessageQueue::reply(MessageQueueMessage* message) {
	if (this->lastPartner != 0) {
		return sendMessage(this->lastPartner, message, this->getId());
	} else {
		//TODO: Good returnCode
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessage* message,
		MessageQueueId_t* receivedFrom) {
	ReturnValue_t status = this->receiveMessage(message);
	*receivedFrom = this->lastPartner;
	return status;
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessage* message) {
	rtems_status_code status = rtems_message_queue_receive(id,
			message->getBuffer(), &(message->messageSize),
			RTEMS_NO_WAIT, 1);
	if (status == RTEMS_SUCCESSFUL) {
		this->lastPartner = message->getSender();
		//Check size of incoming message.
		if (message->messageSize < message->getMinimumMessageSize()) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	} else {
		//No message was received. Keep lastPartner anyway, I might send something later.
		//But still, delete packet content.
		memset(message->getData(), 0, message->MAX_DATA_SIZE);
	}
	return RtemsBasic::convertReturnCode(status);
}

MessageQueueId_t MessageQueue::getLastPartner() const {
	return this->lastPartner;
}

ReturnValue_t MessageQueue::flush(uint32_t* count) {
	rtems_status_code status = rtems_message_queue_flush(id, count);
	return RtemsBasic::convertReturnCode(status);
}

MessageQueueId_t MessageQueue::getId() const {
	return this->id;
}

void MessageQueue::setDefaultDestination(MessageQueueId_t defaultDestination) {
	this->defaultDestination = defaultDestination;
}

ReturnValue_t MessageQueue::sendMessage(MessageQueueId_t sendTo,
		MessageQueueMessage* message, MessageQueueId_t sentFrom,
		bool ignoreFault) {

	message->setSender(sentFrom);
	rtems_status_code result = rtems_message_queue_send(sendTo,
			message->getBuffer(), message->messageSize);

	//TODO: Check if we're in ISR.
	if (result != RTEMS_SUCCESSFUL && !ignoreFault) {
		if (internalErrorReporter == NULL) {
			internalErrorReporter = objectManager->get<InternalErrorReporterIF>(
					objects::INTERNAL_ERROR_REPORTER);
		}
		if (internalErrorReporter != NULL) {
			internalErrorReporter->queueMessageNotSent();
		}
	}
	return result;
}

ReturnValue_t MessageQueue::sendToDefault(MessageQueueMessage* message,
		MessageQueueId_t sentFrom, bool ignoreFault) {
	return sendMessage(defaultDestination, message, sentFrom, ignoreFault);
}

MessageQueueId_t MessageQueue::getDefaultDestination() const {
	return this->defaultDestination;
}

bool MessageQueue::isDefaultDestinationSet() const {
	return (defaultDestination != NO_QUEUE);
}

uint16_t MessageQueue::queueCounter = 0;
