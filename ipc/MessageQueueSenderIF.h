#ifndef FRAMEWORK_IPC_MESSAGEQUEUESENDERIF_H_
#define FRAMEWORK_IPC_MESSAGEQUEUESENDERIF_H_

#include "../ipc/MessageQueueMessageIF.h"
#include "../objectmanager/ObjectManagerIF.h"

class MessageQueueSenderIF {
public:

	virtual ~MessageQueueSenderIF() {}

	/**
	 * Allows sending messages without actually "owning" a message queue.
	 * Not sure whether this is actually a good idea.
	 */
	static ReturnValue_t sendMessage(MessageQueueId_t sendTo,
			MessageQueueMessageIF* message,
			MessageQueueId_t sentFrom = MessageQueueMessageIF::NO_QUEUE,
			bool ignoreFault = false);
private:
	MessageQueueSenderIF() {}
};


#endif /* FRAMEWORK_IPC_MESSAGEQUEUESENDERIF_H_ */
