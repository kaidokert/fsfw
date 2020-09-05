#include "MessageQueue.h"
#include "QueueMapManager.h"

#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../ipc/MutexFactory.h"
#include "../../ipc/MutexHelper.h"

MessageQueue::MessageQueue(size_t messageDepth, size_t maxMessageSize):
		messageSize(maxMessageSize), messageDepth(messageDepth) {
	queueLock = MutexFactory::instance()->createMutex();
	auto result = QueueMapManager::instance()->addMessageQueue(this, &mqId);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "MessageQueue::MessageQueue:"
		        << " Could not be created" << std::endl;
	}
}

MessageQueue::~MessageQueue() {
	MutexFactory::instance()->deleteMutex(queueLock);
}

ReturnValue_t MessageQueue::sendMessage(MessageQueueId_t sendTo,
		MessageQueueMessageIF* message, bool ignoreFault) {
	return sendMessageFrom(sendTo, message, this->getId(), ignoreFault);
}

ReturnValue_t MessageQueue::sendToDefault(MessageQueueMessageIF* message) {
	return sendToDefaultFrom(message, this->getId());
}

ReturnValue_t MessageQueue::sendToDefaultFrom(MessageQueueMessageIF* message,
		MessageQueueId_t sentFrom, bool ignoreFault) {
	return sendMessageFrom(defaultDestination,message,sentFrom,ignoreFault);
}

ReturnValue_t MessageQueue::reply(MessageQueueMessageIF* message) {
	if (this->lastPartner != 0) {
		return sendMessageFrom(this->lastPartner, message, this->getId());
	} else {
		return MessageQueueIF::NO_REPLY_PARTNER;
	}
}

ReturnValue_t MessageQueue::sendMessageFrom(MessageQueueId_t sendTo,
		MessageQueueMessageIF* message, MessageQueueId_t sentFrom,
		bool ignoreFault) {
	return sendMessageFromMessageQueue(sendTo, message, sentFrom,
			ignoreFault);
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessageIF* message,
		MessageQueueId_t* receivedFrom) {
	ReturnValue_t status = this->receiveMessage(message);
	if(status == HasReturnvaluesIF::RETURN_OK) {
		*receivedFrom = this->lastPartner;
	}
	return status;
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessageIF* message) {
	if(messageQueue.empty()) {
		return MessageQueueIF::EMPTY;
	}
	// not sure this will work..
	//*message = std::move(messageQueue.front());
	MutexHelper mutexLock(queueLock, MutexIF::TimeoutType::WAITING, 20);
	MessageQueueMessage* currentMessage = &messageQueue.front();
	std::copy(currentMessage->getBuffer(),
			currentMessage->getBuffer() + messageSize, message->getBuffer());
	messageQueue.pop();
	// The last partner is the first uint32_t field in the message
	this->lastPartner = message->getSender();
	return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t MessageQueue::getLastPartner() const {
	return lastPartner;
}

ReturnValue_t MessageQueue::flush(uint32_t* count) {
	*count = messageQueue.size();
	// Clears the queue.
	messageQueue = std::queue<MessageQueueMessage>();
	return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t MessageQueue::getId() const {
	return mqId;
}

void MessageQueue::setDefaultDestination(MessageQueueId_t defaultDestination) {
	defaultDestinationSet = true;
	this->defaultDestination = defaultDestination;
}

MessageQueueId_t MessageQueue::getDefaultDestination() const {
	return defaultDestination;
}

bool MessageQueue::isDefaultDestinationSet() const {
	return defaultDestinationSet;
}


// static core function to send messages.
ReturnValue_t MessageQueue::sendMessageFromMessageQueue(MessageQueueId_t sendTo,
        MessageQueueMessageIF* message, MessageQueueId_t sentFrom,
        bool ignoreFault) {
	if(message->getMessageSize() > message->getMaximumMessageSize()) {
		// Actually, this should never happen or an error will be emitted
		// in MessageQueueMessage.
		// But I will still return a failure here.
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	MessageQueue* targetQueue = dynamic_cast<MessageQueue*>(
			QueueMapManager::instance()->getMessageQueue(sendTo));
	if(targetQueue == nullptr) {
		if(not ignoreFault) {
			InternalErrorReporterIF* internalErrorReporter =
					objectManager->get<InternalErrorReporterIF>(
							objects::INTERNAL_ERROR_REPORTER);
			if (internalErrorReporter != nullptr) {
				internalErrorReporter->queueMessageNotSent();
			}
		}
		// TODO: Better returnvalue
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	if(targetQueue->messageQueue.size() < targetQueue->messageDepth) {
		MutexHelper mutexLock(targetQueue->queueLock,
		        MutexIF::TimeoutType::WAITING, 20);
		// not ideal, works for now though.
		MessageQueueMessage* mqmMessage =
				dynamic_cast<MessageQueueMessage*>(message);
		if(message != nullptr) {
			targetQueue->messageQueue.push(*mqmMessage);
		}
		else {
			sif::error << "MessageQueue::sendMessageFromMessageQueue: Message"
					"is not MessageQueueMessage!" << std::endl;
		}

	}
	else {
		return MessageQueueIF::FULL;
	}
    message->setSender(sentFrom);
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MessageQueue::lockQueue(MutexIF::TimeoutType timeoutType,
        dur_millis_t lockTimeout) {
	return queueLock->lockMutex(timeoutType, lockTimeout);
}

ReturnValue_t MessageQueue::unlockQueue() {
	return queueLock->unlockMutex();
}
