#ifndef FSFW_EVENTS_EVENTMANAGERIF_H_
#define FSFW_EVENTS_EVENTMANAGERIF_H_

#include "../ipc/MessageQueueIF.h"
#include "../ipc/MessageQueueSenderIF.h"
#include "../objectmanager/ObjectManager.h"
#include "../serviceinterface/ServiceInterface.h"
#include "EventMessage.h"
#include "eventmatching/eventmatching.h"

class EventManagerIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::EVENT_MANAGER_IF;
  static const ReturnValue_t LISTENER_NOT_FOUND = MAKE_RETURN_CODE(1);
  virtual ~EventManagerIF() {}

  virtual MessageQueueId_t getEventReportQueue() = 0;

  virtual ReturnValue_t registerListener(MessageQueueId_t listener,
                                         bool forwardAllButSelected = false) = 0;
  virtual ReturnValue_t subscribeToEvent(MessageQueueId_t listener, EventId_t event) = 0;
  virtual ReturnValue_t subscribeToAllEventsFrom(MessageQueueId_t listener, object_id_t object) = 0;
  virtual ReturnValue_t unsubscribeFromAllEvents(MessageQueueId_t listener, object_id_t object) = 0;
  virtual ReturnValue_t subscribeToEventRange(MessageQueueId_t listener, EventId_t idFrom = 0,
                                              EventId_t idTo = 0, bool idInverted = false,
                                              object_id_t reporterFrom = 0,
                                              object_id_t reporterTo = 0,
                                              bool reporterInverted = false) = 0;
  virtual ReturnValue_t unsubscribeFromEventRange(MessageQueueId_t listener, EventId_t idFrom = 0,
                                                  EventId_t idTo = 0, bool idInverted = false,
                                                  object_id_t reporterFrom = 0,
                                                  object_id_t reporterTo = 0,
                                                  bool reporterInverted = false) = 0;

  static void triggerEvent(object_id_t reportingObject, Event event, uint32_t parameter1 = 0,
                           uint32_t parameter2 = 0, MessageQueueId_t sentFrom = 0) {
    EventMessage message(event, reportingObject, parameter1, parameter2);
    triggerEvent(&message, sentFrom);
  }

  static void triggerEvent(EventMessage* message, MessageQueueId_t sentFrom = 0) {
    if (eventmanagerQueue == MessageQueueIF::NO_QUEUE) {
      EventManagerIF* eventmanager =
          ObjectManager::instance()->get<EventManagerIF>(objects::EVENT_MANAGER);
      if (eventmanager == nullptr) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "EventManagerIF::triggerEvent: EventManager invalid or not found!"
                     << std::endl;
#else
        sif::printWarning(
            "EventManagerIF::triggerEvent: "
            "EventManager invalid or not found!");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
        return;
      }
      eventmanagerQueue = eventmanager->getEventReportQueue();
    }
    MessageQueueSenderIF::sendMessage(eventmanagerQueue, message, sentFrom);
  }

 private:
  //! Initialized by EventManager (C++11 does not allow header-only static member initialization).
  static MessageQueueId_t eventmanagerQueue;
};

#endif /* FSFW_EVENTS_EVENTMANAGERIF_H_ */
