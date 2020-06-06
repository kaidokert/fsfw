#ifndef FRAMEWORK_IPC_MESSAGEQUEUESENDERIF_H_
#define FRAMEWORK_IPC_MESSAGEQUEUESENDERIF_H_

#include <framework/objectmanager/ObjectManagerIF.h>
class MessageQueueMessage;


//TODO: Actually, the definition of this ID to be a uint32_t is not ideal and breaks layering.
//However, it is difficult to keep layering, as the ID is stored in many places and sent around in
//MessageQueueMessage.
//Ideally, one would use the (current) object_id_t only, however, doing a lookup of queueIDs for every
//call does not sound ideal.
//In a first step, I'll circumvent the issue by not touching it, maybe in a second step.
//This also influences Interface design (getCommandQueue) and some other issues..
typedef uint32_t MessageQueueId_t;

class MessageQueueSenderIF {
public:
	static const MessageQueueId_t NO_QUEUE = 0xffffffff;

	virtual ~MessageQueueSenderIF() {}

	/**
	 * Allows sending messages without actually "owing" a message queue.
	 * Not sure whether this is actually a good idea.
	 * Must be implemented by a subclass.
	 */
	static ReturnValue_t sendMessage(MessageQueueId_t sendTo,
			MessageQueueMessage* message, MessageQueueId_t sentFrom = MessageQueueSenderIF::NO_QUEUE,
			bool ignoreFault=false);
private:
	MessageQueueSenderIF() {}
};



#endif /* FRAMEWORK_IPC_MESSAGEQUEUESENDERIF_H_ */
