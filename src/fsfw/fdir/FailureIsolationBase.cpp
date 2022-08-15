#include "fsfw/fdir/FailureIsolationBase.h"

#include "fsfw/events/EventManagerIF.h"
#include "fsfw/health/HasHealthIF.h"
#include "fsfw/health/HealthMessage.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"

FailureIsolationBase::FailureIsolationBase(object_id_t owner, object_id_t parent,
                                           uint8_t messageDepth, uint8_t parameterDomainBase)
    : ownerId(owner), faultTreeParent(parent), parameterDomainBase(parameterDomainBase) {
  eventQueue =
      QueueFactory::instance()->createMessageQueue(messageDepth, EventMessage::EVENT_MESSAGE_SIZE);
}

FailureIsolationBase::~FailureIsolationBase() {
  EventManagerIF* manager = ObjectManager::instance()->get<EventManagerIF>(objects::EVENT_MANAGER);
  if (manager == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "FailureIsolationBase::~FailureIsolationBase: Event Manager has not"
                  " been initialized!"
               << std::endl;
#endif
    return;
  }
  manager->unsubscribeFromAllEvents(eventQueue->getId(), ownerId);
  QueueFactory::instance()->deleteMessageQueue(eventQueue);
}

ReturnValue_t FailureIsolationBase::initialize() {
  EventManagerIF* manager = ObjectManager::instance()->get<EventManagerIF>(objects::EVENT_MANAGER);
  if (manager == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "FailureIsolationBase::initialize: Event Manager has not"
                  " been initialized!"
               << std::endl;
#endif
    return returnvalue::FAILED;
  }
  ReturnValue_t result = manager->registerListener(eventQueue->getId());
  if (result != returnvalue::OK) {
    return result;
  }
  if (ownerId != objects::NO_OBJECT) {
    result = manager->subscribeToAllEventsFrom(eventQueue->getId(), ownerId);
    if (result != returnvalue::OK) {
      return result;
    }
    owner = ObjectManager::instance()->get<HasHealthIF>(ownerId);
    if (owner == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "FailureIsolationBase::intialize: Owner object "
                    "invalid. Make sure it implements HasHealthIF"
                 << std::endl;
#endif
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  }
  if (faultTreeParent != objects::NO_OBJECT) {
    ConfirmsFailuresIF* parentIF =
        ObjectManager::instance()->get<ConfirmsFailuresIF>(faultTreeParent);
    if (parentIF == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "FailureIsolationBase::intialize: Parent object"
                 << "invalid." << std::endl;
#endif
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "Make sure it implements ConfirmsFailuresIF." << std::endl;
#endif
      return ObjectManagerIF::CHILD_INIT_FAILED;
      return returnvalue::FAILED;
    }
    eventQueue->setDefaultDestination(parentIF->getEventReceptionQueue());
  }
  return returnvalue::OK;
}

void FailureIsolationBase::checkForFailures() {
  EventMessage event;
  for (ReturnValue_t result = eventQueue->receiveMessage(&event); result == returnvalue::OK;
       result = eventQueue->receiveMessage(&event)) {
    if (event.getSender() == eventQueue->getId()) {
      // We already got this event, because we sent it.
      continue;
    }
    switch (event.getMessageId()) {
      case EventMessage::EVENT_MESSAGE:
        if (isFdirDisabledForSeverity(event.getSeverity())) {
          // We do not handle events when disabled.
          continue;
        }
        eventReceived(&event);
        break;
      case EventMessage::CONFIRMATION_REQUEST:
        doConfirmFault(&event);
        break;
      case EventMessage::YOUR_FAULT:
        eventConfirmed(&event);
        break;
      case EventMessage::MY_FAULT:
        wasParentsFault(&event);
        break;
      default:
        break;
    }
  }
  decrementFaultCounters();
}

void FailureIsolationBase::setOwnerHealth(HasHealthIF::HealthState health) {
  if (owner != NULL) {
    owner->setHealth(health);
  }
  // else: owner has no health.
}

MessageQueueId_t FailureIsolationBase::getEventReceptionQueue() { return eventQueue->getId(); }

ReturnValue_t FailureIsolationBase::sendConfirmationRequest(EventMessage* event,
                                                            MessageQueueId_t destination) {
  event->setMessageId(EventMessage::CONFIRMATION_REQUEST);
  if (destination != MessageQueueIF::NO_QUEUE) {
    return eventQueue->sendMessage(destination, event);
  } else if (faultTreeParent != objects::NO_OBJECT) {
    return eventQueue->sendToDefault(event);
  }
  return returnvalue::FAILED;
}

void FailureIsolationBase::eventConfirmed(EventMessage* event) {}

void FailureIsolationBase::wasParentsFault(EventMessage* event) {}

void FailureIsolationBase::doConfirmFault(EventMessage* event) {
  ReturnValue_t result = confirmFault(event);
  if (result == YOUR_FAULT) {
    event->setMessageId(EventMessage::YOUR_FAULT);
    eventQueue->reply(event);
  } else if (result == MY_FAULT) {
    event->setMessageId(EventMessage::MY_FAULT);
    eventQueue->reply(event);
  } else {
  }
}

ReturnValue_t FailureIsolationBase::confirmFault(EventMessage* event) { return YOUR_FAULT; }

void FailureIsolationBase::triggerEvent(Event event, uint32_t parameter1, uint32_t parameter2) {
  // With this mechanism, all events are disabled for a certain device.
  // That's not so good for visibility.
  if (isFdirDisabledForSeverity(event::getSeverity(event))) {
    return;
  }
  EventMessage message(event, ownerId, parameter1, parameter2);
  EventManagerIF::triggerEvent(&message, eventQueue->getId());
  eventReceived(&message);
}

bool FailureIsolationBase::isFdirDisabledForSeverity(EventSeverity_t severity) {
  if ((owner != NULL) && (severity != severity::INFO)) {
    if (owner->getHealth() == HasHealthIF::EXTERNAL_CONTROL) {
      // External control disables handling of fault messages.
      return true;
    }
  }
  return false;
}

void FailureIsolationBase::throwFdirEvent(Event event, uint32_t parameter1, uint32_t parameter2) {
  EventMessage message(event, ownerId, parameter1, parameter2);
  EventManagerIF::triggerEvent(&message, eventQueue->getId());
}
