#ifndef FSFW_MEMORY_ACCEPTSMEMORYMESSAGESIF_H_
#define FSFW_MEMORY_ACCEPTSMEMORYMESSAGESIF_H_

#include "HasMemoryIF.h"
#include "MemoryMessage.h"
#include "../ipc/MessageQueueSenderIF.h"

class AcceptsMemoryMessagesIF : public HasMemoryIF {
public:
	virtual MessageQueueId_t getCommandQueue() const = 0;
};


#endif /* FSFW_MEMORY_ACCEPTSMEMORYMESSAGESIF_H_ */
