#ifndef HEALTHDEVICE_H_
#define HEALTHDEVICE_H_

#include "../health/HasHealthIF.h"
#include "../health/HealthHelper.h"
#include "../objectmanager/SystemObject.h"
#include "../tasks/ExecutableObjectIF.h"
#include "../ipc/MessageQueueIF.h"

class HealthDevice: public SystemObject,
		public ExecutableObjectIF,
		public HasHealthIF {
public:
	HealthDevice(object_id_t setObjectId, MessageQueueId_t parentQueue);
	virtual ~HealthDevice();

	ReturnValue_t performOperation(uint8_t opCode);

	ReturnValue_t initialize();

	virtual MessageQueueId_t getCommandQueue() const;

	void setParentQueue(MessageQueueId_t parentQueue);

	bool hasHealthChanged();

	virtual ReturnValue_t setHealth(HealthState health);

	virtual HealthState getHealth();

protected:
	HealthState lastHealth;

	MessageQueueId_t parentQueue;
	MessageQueueIF* commandQueue;
public:
	HealthHelper healthHelper;
};

#endif /* HEALTHDEVICE_H_ */
