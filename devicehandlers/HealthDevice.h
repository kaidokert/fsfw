#ifndef HEALTHDEVICE_H_
#define HEALTHDEVICE_H_

#include <framework/health/HasHealthIF.h>
#include <framework/health/HealthHelper.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/tasks/ExecutableObjectIF.h>

class HealthDevice: public SystemObject,
		public ExecutableObjectIF,
		public HasHealthIF {
public:
	HealthDevice(object_id_t setObjectId, MessageQueueId_t parentQueue);
	virtual ~HealthDevice();

	ReturnValue_t performOperation();

	ReturnValue_t initialize();

	virtual MessageQueueId_t getCommandQueue() const;

	void setParentQueue(MessageQueueId_t parentQueue);

	bool hasHealthChanged();

	virtual ReturnValue_t setHealth(HealthState health);

	virtual HealthState getHealth();

protected:
	HealthState lastHealth;

	MessageQueueId_t parentQueue;
	MessageQueue commandQueue;
public:
	HealthHelper healthHelper;
};

#endif /* HEALTHDEVICE_H_ */
