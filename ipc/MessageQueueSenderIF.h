#ifndef FRAMEWORK_IPC_MESSAGEQUEUESENDERIF_H_
#define FRAMEWORK_IPC_MESSAGEQUEUESENDERIF_H_

#include <framework/ipc/MessageQueueMessageIF.h>
#include <framework/objectmanager/ObjectManagerIF.h>

class MessageQueueSenderIF {
public:

	virtual ~MessageQueueSenderIF() {}

	/**
	 * Allows sending messages without actually "owing" a message queue.
	 * Not sure whether this is actually a good idea.
	 * Must be implemented by a subclass.
	 */
	static ReturnValue_t sendMessage(MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, size_t maxMessageSize,
			MessageQueueId_t sentFrom = MessageQueueMessageIF::NO_QUEUE,
			bool ignoreFault=false);
private:
	MessageQueueSenderIF() {}
};



#endif /* FRAMEWORK_IPC_MESSAGEQUEUESENDERIF_H_ */
