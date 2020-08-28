/**
 * @file	AcceptsMemoryMessagesIF.h
 * @brief	This file defines the AcceptsMemoryMessagesIF class.
 * @date	11.07.2013
 * @author	baetz
 */

#ifndef ACCEPTSMEMORYMESSAGESIF_H_
#define ACCEPTSMEMORYMESSAGESIF_H_

#include "../memory/HasMemoryIF.h"
#include "../memory/MemoryMessage.h"
#include "../ipc/MessageQueueSenderIF.h"

class AcceptsMemoryMessagesIF : public HasMemoryIF {
public:
	virtual MessageQueueId_t getCommandQueue() const = 0;
};


#endif /* ACCEPTSMEMORYMESSAGESIF_H_ */
