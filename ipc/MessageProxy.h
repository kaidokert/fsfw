/*
 * MessageProxy.h
 *
 *  Created on: 18.03.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_IPC_MESSAGEPROXY_H_
#define FRAMEWORK_IPC_MESSAGEPROXY_H_

#include <framework/container/FIFO.h>
#include <framework/ipc/CommandMessage.h>
#include <framework/ipc/MessageQueue.h>
/**
 * Simple "one message forwarding" proxy.
 * Could be extended to forwarding to multiple recipients in parallel
 * with a small forwarding map.
 */
class MessageProxy {
public:
	MessageProxy(size_t queueDepth = DEFAULT_QUEUE_DEPTH, MessageQueueId_t setReceiver = 0);
	virtual ~MessageProxy();
	MessageQueueId_t getCommandQueue() const;
	MessageQueueId_t getReceiver() const;
	void setReceiver(MessageQueueId_t configuredReceiver);
	/**
	 * Checks the commandQueue for commands from other stuff or replies from the Receiver.
	 * There's the implicit assumption, that we get commands from anywhere, but replies only from the
	 * #configuredReceiver.
	 */
	void checkCommandQueue();
	void flush();
	static const uint8_t INTERFACE_ID = MESSAGE_PROXY;
	static const ReturnValue_t FLUSHED = MAKE_RETURN_CODE(1);
private:
	static const size_t DEFAULT_QUEUE_DEPTH = 5;
	MessageQueueId_t receiver;
	MessageQueueId_t currentRequest;
	MessageQueue commandQueue;
	FIFO<CommandMessage, DEFAULT_QUEUE_DEPTH> commandFifo; //!< Required because there might be small bursts of commands coming in parallel.
};

#endif /* FRAMEWORK_IPC_MESSAGEPROXY_H_ */
