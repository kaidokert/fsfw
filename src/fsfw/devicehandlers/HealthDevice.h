#ifndef FSFW_DEVICEHANDLERS_HEALTHDEVICE_H_
#define FSFW_DEVICEHANDLERS_HEALTHDEVICE_H_

#include "fsfw/health/HasHealthIF.h"
#include "fsfw/health/HealthHelper.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

class HealthDevice : public SystemObject, public ExecutableObjectIF, public HasHealthIF {
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

#endif /* FSFW_DEVICEHANDLERS_HEALTHDEVICE_H_ */
