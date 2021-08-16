#ifndef FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_
#define FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_

#include "fsfw_tests/unit/CatchDefinitions.h"

#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/MessageQueueMessage.h"

#include <cstring>

#include <queue>

class MessageQueueMockBase: public MessageQueueIF {
public:
	MessageQueueId_t myQueueId = tconst::testQueueId;
	uint8_t messageSentCounter = 0;
	bool defaultDestSet = false;
	bool messageSent = false;


	bool wasMessageSent(uint8_t* messageSentCounter = nullptr,
			bool resetCounter = true) {
		bool tempMessageSent = messageSent;
		messageSent = false;
		if(messageSentCounter != nullptr) {
			*messageSentCounter = this->messageSentCounter;
		}
		if(resetCounter) {
			this->messageSentCounter = 0;
		}
		return tempMessageSent;
	}

	/**
	 * Pop a message, clearing it in the process.
	 * @return
	 */
	ReturnValue_t popMessage() {
	    CommandMessage message;
	    message.clear();
	    return receiveMessage(&message);
	}

	virtual ReturnValue_t reply( MessageQueueMessageIF* message ) {
		return sendMessage(myQueueId, message);
	};
	virtual ReturnValue_t receiveMessage(MessageQueueMessageIF* message,
			MessageQueueId_t *receivedFrom) {
		return receiveMessage(message);
	}

	virtual ReturnValue_t receiveMessage(MessageQueueMessageIF* message) {
		if(messagesSentQueue.empty()) {
			return MessageQueueIF::EMPTY;
		}

		std::memcpy(message->getBuffer(), messagesSentQueue.front().getBuffer(),
				message->getMessageSize());
		messagesSentQueue.pop();
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual ReturnValue_t flush(uint32_t* count) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	virtual MessageQueueId_t getLastPartner() const {
		return myQueueId;
	}
	virtual MessageQueueId_t getId() const {
		return myQueueId;
	}
	virtual ReturnValue_t sendMessageFrom( MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, MessageQueueId_t sentFrom,
			bool ignoreFault = false ) {
		return sendMessage(sendTo, message);
	}
	virtual ReturnValue_t sendToDefaultFrom( MessageQueueMessageIF* message,
			MessageQueueId_t sentFrom, bool ignoreFault = false ) {
		return sendMessage(myQueueId, message);
	}
	virtual ReturnValue_t sendToDefault( MessageQueueMessageIF* message ) {
		return sendMessage(myQueueId, message);
	}
	virtual ReturnValue_t sendMessage( MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, bool ignoreFault = false ) override {
		messageSent = true;
		messageSentCounter++;
		MessageQueueMessage& messageRef = *(
				dynamic_cast<MessageQueueMessage*>(message));
		messagesSentQueue.push(messageRef);
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

	void clearMessages(bool clearCommandMessages = true) {
		while(not messagesSentQueue.empty()) {
			if(clearCommandMessages) {
				CommandMessage message;
				std::memcpy(message.getBuffer(),
						messagesSentQueue.front().getBuffer(),
						message.getMessageSize());
				message.clear();
			}
			messagesSentQueue.pop();
		}
	}

private:
	std::queue<MessageQueueMessage> messagesSentQueue;
};


#endif /* FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_ */
