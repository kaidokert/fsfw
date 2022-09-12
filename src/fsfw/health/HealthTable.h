#ifndef FSFW_HEALTH_HEALTHTABLE_H_
#define FSFW_HEALTH_HEALTHTABLE_H_

#include <map>

#include "../ipc/MutexIF.h"
#include "../objectmanager/SystemObject.h"
#include "HealthTableIF.h"

class HealthTable : public HealthTableIF, public SystemObject {
 public:
  explicit HealthTable(object_id_t objectid);
  ~HealthTable() override;

  void setMutexTimeout(MutexIF::TimeoutType timeoutType, uint32_t timeoutMs);

  /** HealthTableIF overrides */
  virtual ReturnValue_t registerObject(
      object_id_t object, HasHealthIF::HealthState initilialState = HasHealthIF::HEALTHY) override;
  ReturnValue_t removeObject(object_id_t object) override;
  virtual size_t getPrintSize() override;
  virtual void printAll(uint8_t* pointer, size_t maxSize) override;

  /** ManagesHealthIF overrides */
  virtual bool hasHealth(object_id_t object) override;
  virtual void setHealth(object_id_t object, HasHealthIF::HealthState newState) override;
  virtual HasHealthIF::HealthState getHealth(object_id_t) override;

 protected:
  using HealthMap = std::map<object_id_t, HasHealthIF::HealthState>;
  using HealthEntry = std::pair<object_id_t, HasHealthIF::HealthState>;

  MutexIF* mutex;
  MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING;
  uint32_t mutexTimeoutMs = 20;

  HealthMap healthMap;

  HealthMap::iterator mapIterator;

  virtual ReturnValue_t iterate(HealthEntry* value, bool reset = false) override;
};

#endif /* FSFW_HEALTH_HEALTHTABLE_H_ */
