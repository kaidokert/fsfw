#ifndef FRAMEWORK_IPC_QUEUEFACTORY_H_
#define FRAMEWORK_IPC_QUEUEFACTORY_H_

#include <framework/ipc/MessageQueueIF.h>
#include <cstdint>
/**
 * Creates message queues.
 * This class is a "singleton" interface, i.e. it provides an
 * interface, but also is the base class for a singleton.
 */
class QueueFactory {
public:
	virtual ~QueueFactory();
	/**
	 * Returns the single instance of QueueFactory.
	 * The implementation of #instance is found in its subclasses.
	 * Thus, we choose link-time variability of the  instance.
	 */
	static QueueFactory* instance();

	MessageQueueIF* createMessageQueue(size_t messageDepth = 3,
			size_t maxMessageSize = MessageQueueMessage::MAX_MESSAGE_SIZE);

	void deleteMessageQueue(MessageQueueIF* queue);
private:
	/**
	 * External instantiation is not allowed.
	 */
	QueueFactory();
	static QueueFactory* factoryInstance;
};

#endif /* FRAMEWORK_IPC_QUEUEFACTORY_H_ */
