#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <framework/ipc/MessageQueue.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/tasks/ExecutableObjectIF.h>
#include <framework/watchdog/WatchdogMessage.h>

class Watchdog: public ExecutableObjectIF, public SystemObject {
public:
	Watchdog(object_id_t objectId, uint32_t interval_us, uint32_t initial_interval_us);
	virtual ~Watchdog();
	virtual ReturnValue_t performOperation();

	MessageQueueId_t getCommandQueue();
protected:
	MessageQueue commandQueue;
};

#endif /* WATCHDOG_H_ */
