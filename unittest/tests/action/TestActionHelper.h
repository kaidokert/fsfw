//#ifndef UNITTEST_HOSTED_TESTACTIONHELPER_H_
//#define UNITTEST_HOSTED_TESTACTIONHELPER_H_
//
//#include <fsfw/action/HasActionsIF.h>
//#include <fsfw/ipc/MessageQueueIF.h>
//#include <fsfw/unittest/core/CatchDefinitions.h>
//#include <cstring>
//
//
//class ActionHelperOwnerMockBase: public HasActionsIF {
//public:
//	bool getCommandQueueCalled = false;
//	bool executeActionCalled = false;
//	static const size_t MAX_SIZE = 3;
//	uint8_t buffer[MAX_SIZE] = {0, 0, 0};
//	size_t size = 0;
//
//	MessageQueueId_t getCommandQueue() const override {
//		return tconst::testQueueId;
//	}
//
//	ReturnValue_t executeAction(ActionId_t actionId, MessageQueueId_t commandedBy,
//			const uint8_t* data, size_t size) override {
//		executeActionCalled = true;
//		if(size > MAX_SIZE){
//			return 0xAFFE;
//		}
//		this->size = size;
//		memcpy(buffer, data, size);
//		return HasReturnvaluesIF::RETURN_OK;
//	}
//
//	void clearBuffer(){
//		this->size = 0;
//		for(size_t i = 0; i<MAX_SIZE; i++){
//			buffer[i] = 0;
//		}
//	}
//
//	void getBuffer(const uint8_t** ptr, size_t* size){
//		if(size != nullptr){
//			*size = this->size;
//		}
//		if(ptr != nullptr){
//			*ptr = buffer;
//		}
//	}
//};
//
//
//class MessageQueueMockBase: public MessageQueueIF {
//public:
//	MessageQueueId_t myQueueId = 0;
//	bool defaultDestSet = false;
//	bool messageSent = false;
//
//
//
//	bool wasMessageSent() {
//		bool tempMessageSent = messageSent;
//		messageSent = false;
//		return tempMessageSent;
//	}
//
//	virtual ReturnValue_t reply( MessageQueueMessage* message ) {
//		messageSent = true;
//		lastMessage = (*message);
//		return HasReturnvaluesIF::RETURN_OK;
//	};
//	virtual ReturnValue_t receiveMessage(MessageQueueMessage* message,
//			MessageQueueId_t *receivedFrom) {
//		(*message) = lastMessage;
//		lastMessage.clear();
//		return HasReturnvaluesIF::RETURN_OK;
//	}
//	virtual ReturnValue_t receiveMessage(MessageQueueMessage* message) {
//		(*message) = lastMessage;
//		lastMessage.clear();
//		return HasReturnvaluesIF::RETURN_OK;
//	}
//	virtual ReturnValue_t flush(uint32_t* count) {
//		return HasReturnvaluesIF::RETURN_OK;
//	}
//	virtual MessageQueueId_t getLastPartner() const {
//		return tconst::testQueueId;
//	}
//	virtual MessageQueueId_t getId() const {
//		return tconst::testQueueId;
//	}
//	virtual ReturnValue_t sendMessageFrom( MessageQueueId_t sendTo,
//			MessageQueueMessage* message, MessageQueueId_t sentFrom,
//			bool ignoreFault = false ) {
//		messageSent = true;
//		lastMessage = (*message);
//		return HasReturnvaluesIF::RETURN_OK;
//	}
//	virtual ReturnValue_t sendMessage( MessageQueueId_t sendTo,
//			MessageQueueMessage* message, bool ignoreFault = false ) override {
//		messageSent = true;
//		lastMessage = (*message);
//		return HasReturnvaluesIF::RETURN_OK;
//	}
//	virtual ReturnValue_t sendToDefaultFrom( MessageQueueMessage* message,
//			MessageQueueId_t sentFrom, bool ignoreFault = false ) {
//		messageSent = true;
//		lastMessage = (*message);
//		return HasReturnvaluesIF::RETURN_OK;
//	}
//	virtual ReturnValue_t sendToDefault( MessageQueueMessage* message ) {
//		messageSent = true;
//		lastMessage = (*message);
//		return HasReturnvaluesIF::RETURN_OK;
//	}
//	virtual void setDefaultDestination(MessageQueueId_t defaultDestination) {
//		myQueueId = defaultDestination;
//		defaultDestSet = true;
//	}
//
//	virtual MessageQueueId_t getDefaultDestination() const {
//		return myQueueId;
//	}
//	virtual bool isDefaultDestinationSet() const {
//		return defaultDestSet;
//	}
//private:
//	MessageQueueMessage lastMessage;
//
//};
//
//
//#endif /* UNITTEST_TESTFW_NEWTESTS_TESTACTIONHELPER_H_ */
