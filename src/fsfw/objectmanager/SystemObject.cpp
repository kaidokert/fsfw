#include "fsfw/objectmanager/SystemObject.h"

#include "fsfw/events/EventManagerIF.h"
#include "fsfw/objectmanager/ObjectManager.h"

SystemObject::SystemObject(object_id_t setObjectId, bool doRegister)
    : objectId(setObjectId), registered(doRegister) {
  if (registered) {
    ObjectManager::instance()->insert(objectId, this);
  }
}

SystemObject::~SystemObject() {
  if (registered) {
    ObjectManager::instance()->remove(objectId);
  }
}

object_id_t SystemObject::getObjectId() const { return objectId; }

void SystemObject::triggerEvent(Event event, uint32_t parameter1, uint32_t parameter2) {
  EventManagerIF::triggerEvent(objectId, event, parameter1, parameter2);
}

ReturnValue_t SystemObject::initialize() { return returnvalue::OK; }

ReturnValue_t SystemObject::checkObjectConnections() { return returnvalue::OK; }

void SystemObject::forwardEvent(Event event, uint32_t parameter1, uint32_t parameter2) const {
  EventManagerIF::triggerEvent(objectId, event, parameter1, parameter2);
}
