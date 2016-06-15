/*
 * MemoryProxyIF.h
 *
 *  Created on: 18.03.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_MEMORY_MEMORYPROXYIF_H_
#define FRAMEWORK_MEMORY_MEMORYPROXYIF_H_

#include <framework/memory/AcceptsMemoryMessagesIF.h>

/**
 * This was a nice idea to transparently forward incoming messages to another object.
 * But it doesn't work like that.
 */
class MemoryProxyIF : public AcceptsMemoryMessagesIF {
public:
	virtual MessageQueueId_t getProxyQueue() const = 0;
	MessageQueueId_t getCommandQueue() const {
		return getProxyQueue();
	}
	virtual ~MemoryProxyIF() {}

};



#endif /* FRAMEWORK_MEMORY_MEMORYPROXYIF_H_ */
