#ifndef FSFW_HEALTH_HEALTHTABLEIF_H_
#define FSFW_HEALTH_HEALTHTABLEIF_H_

#include <utility>

#include "../objectmanager/ObjectManagerIF.h"
#include "../returnvalues/returnvalue.h"
#include "ManagesHealthIF.h"

class HealthTableIF : public ManagesHealthIF {
 public:
  virtual ~HealthTableIF() {}

  virtual ReturnValue_t registerObject(
      object_id_t object, HasHealthIF::HealthState initilialState = HasHealthIF::HEALTHY) = 0;

  virtual ReturnValue_t removeObject(object_id_t objectId) = 0;

  virtual size_t getPrintSize() = 0;
  virtual void printAll(uint8_t *pointer, size_t maxSize) = 0;

 protected:
  virtual ReturnValue_t iterate(std::pair<object_id_t, HasHealthIF::HealthState> *value,
                                bool reset = false) = 0;
};

#endif /* FRAMEWORK_HEALTH_HEALTHTABLEIF_H_ */
