#ifndef FSFW_PUS_SERVICEPACKETS_SERVICE201PACKETS_H_
#define FSFW_PUS_SERVICEPACKETS_SERVICE201PACKETS_H_

#include "../../health/HasHealthIF.h"
#include "../../serialize/SerialLinkedListAdapter.h"
#include "../../serialize/SerializeIF.h"

class HealthSetCommand
    : public SerialLinkedListAdapter<SerializeIF> {  //!< [EXPORT] : [SUBSERVICE] 1
 public:
  HealthSetCommand() { setLinks(); }

  HasHealthIF::HealthState getHealth() {
    return static_cast<HasHealthIF::HealthState>(health.entry);
  }

 private:
  void setLinks() {
    setStart(&objectId);
    objectId.setNext(&health);
  }
  SerializeElement<uint32_t> objectId;  //!< [EXPORT] : [COMMENT] Target object Id
  SerializeElement<uint8_t> health;     //!< [EXPORT] : [COMMENT] Health to set
};

class HealthSetReply : public SerialLinkedListAdapter<SerializeIF> {  //!< [EXPORT] : [SUBSERVICE] 2
 public:
  HealthSetReply(uint8_t health_, uint8_t oldHealth_) : health(health_), oldHealth(oldHealth_) {
    setLinks();
  }

 private:
  HealthSetReply(const HealthSetReply &reply);
  void setLinks() {
    setStart(&objectId);
    objectId.setNext(&health);
    health.setNext(&oldHealth);
  }
  SerializeElement<uint32_t> objectId;  //!< [EXPORT] : [COMMENT] Source object ID
  SerializeElement<uint8_t> health;     //!< [EXPORT] : [COMMENT] New Health
  SerializeElement<uint8_t> oldHealth;  //!< [EXPORT] : [COMMENT] Old Health
};

#endif /* FSFW_PUS_SERVICEPACKETS_SERVICE201PACKETS_H_ */
