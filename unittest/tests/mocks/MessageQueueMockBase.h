#ifndef FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_
#define FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_

#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/ipc/MessageQueueMessage.h>
#include <unittest/core/CatchDefinitions.h>
#include <cstring>


class MessageQueueMockBase: public MessageQueueIF {
public:
	MessageQueueId_t myQueueId = 0;
	bool defaultDestSet = false;
	bool messageSent = false;


	bool wasMessageSent() {
		bool tempMessageSent = messageSent;
		messageSent = false;
		return tempMessageSent;
	}

	virtual ReturnValue_t reply( MessageQueueMessageIF* message ) {
		messageSent = true;
		lastMessage = *(dynamic_cast<MessageQueueMessage*>(message));
		return HasReturnvaluesIF::RETURN_OK;
	};
	virtual ReturnValue_t receiveMessage(MessageQueueMessageIF* message,
			MessageQueueId_t *receivedFrom) {
		(*message) = lastMessage;
		lastMessage.clear();
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual ReturnValue_t receiveMessage(MessageQueueMessageIF* message) {
		std::memcpy(message->getBuffer(), lastMessage.getBuffer(),
				message->getMessageSize());
		lastMessage.clear();
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual ReturnValue_t flush(uint32_t* count) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual MessageQueueId_t getLastPartner() const {
		return tconst::testQueueId;
	}
	virtual MessageQueueId_t getId() const {
		return tconst::testQueueId;
	}
	virtual ReturnValue_t sendMessageFrom( MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, MessageQueueId_t sentFrom,
			bool ignoreFault = false ) {
		messageSent = true;
		lastMessage = *(dynamic_cast<MessageQueueMessage*>(message));
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual ReturnValue_t sendMessage( MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, bool ignoreFault = false ) override {
		messageSent = true;
		lastMessage = *(dynamic_cast<MessageQueueMessage*>(message));
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual ReturnValue_t sendToDefaultFrom( MessageQueueMessageIF* message,
			MessageQueueId_t sentFrom, bool ignoreFault = false ) {
		messageSent = true;
		lastMessage = *(dynamic_cast<MessageQueueMessage*>(message));
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual ReturnValue_t sendToDefault( MessageQueueMessageIF* message ) {
		messageSent = true;
		lastMessage = *(dynamic_cast<MessageQueueMessage*>(message));
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual void setDefaultDestination(MessageQueueId_t defaultDestination) {
		myQueueId = defaultDestination;
		defaultDestSet = true;
	}

	virtual MessageQueueId_t getDefaultDestination() const {
		return myQueueId;
	}
	virtual bool isDefaultDestinationSet() const {
		return defaultDestSet;
	}
private:
	MessageQueueMessage lastMessage;

};


#endif /* FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_ */
