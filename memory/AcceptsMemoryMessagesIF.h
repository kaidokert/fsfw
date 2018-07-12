/**
 * @file	AcceptsMemoryMessagesIF.h
 * @brief	This file defines the AcceptsMemoryMessagesIF class.
 * @date	11.07.2013
 * @author	baetz
 */

#ifndef ACCEPTSMEMORYMESSAGESIF_H_
#define ACCEPTSMEMORYMESSAGESIF_H_

#include <framework/memory/HasMemoryIF.h>
#include <framework/memory/MemoryMessage.h>
#include <framework/ipc/MessageQueueSenderIF.h>

class AcceptsMemoryMessagesIF : public HasMemoryIF {
public:
	virtual MessageQueueId_t getCommandQueue() const = 0;
};


#endif /* ACCEPTSMEMORYMESSAGESIF_H_ */
