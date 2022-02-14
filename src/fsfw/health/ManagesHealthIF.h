#ifndef FSFW_HEALTH_MANAGESHEALTHIF_H_
#define FSFW_HEALTH_MANAGESHEALTHIF_H_

#include "../objectmanager/ObjectManagerIF.h"
#include "HasHealthIF.h"

class ManagesHealthIF {
 public:
  virtual ~ManagesHealthIF() {}
  virtual bool hasHealth(object_id_t object) = 0;
  virtual void setHealth(object_id_t object, HasHealthIF::HealthState newState) = 0;
  virtual HasHealthIF::HealthState getHealth(object_id_t) = 0;

  virtual bool isHealthy(object_id_t object) { return (getHealth(object) == HasHealthIF::HEALTHY); }

  virtual bool isHealthy(HasHealthIF::HealthState health) {
    return (health == HasHealthIF::HEALTHY);
  }

  virtual bool isFaulty(object_id_t object) {
    HasHealthIF::HealthState health = getHealth(object);
    return isFaulty(health);
  }

  virtual bool isPermanentFaulty(object_id_t object) {
    HasHealthIF::HealthState health = getHealth(object);
    return isPermanentFaulty(health);
  }

  virtual bool isPermanentFaulty(HasHealthIF::HealthState health) {
    return (health == HasHealthIF::PERMANENT_FAULTY);
  }

  static bool isFaulty(HasHealthIF::HealthState health) {
    return ((health == HasHealthIF::FAULTY) || (health == HasHealthIF::PERMANENT_FAULTY) ||
            (health == HasHealthIF::NEEDS_RECOVERY));
  }

  virtual bool isCommandable(object_id_t object) {
    return (getHealth(object) != HasHealthIF::EXTERNAL_CONTROL);
  }

  virtual bool isCommandable(HasHealthIF::HealthState health) {
    return (health != HasHealthIF::EXTERNAL_CONTROL);
  }
};

#endif /* FSFW_HEALTH_MANAGESHEALTHIF_H_ */
