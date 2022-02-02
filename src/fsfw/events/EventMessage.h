#ifndef FSFW_EVENTS_EVENTMESSAGE_H_
#define FSFW_EVENTS_EVENTMESSAGE_H_

#include "Event.h"
#include "fsfw/ipc/MessageQueueMessage.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"

/**
 * Passing on events through IPC.
 * Event Id, severity and message type retrieval is a bit difficult.
 */
class EventMessage : public MessageQueueMessage {
 public:
  static const uint8_t EVENT_MESSAGE = 0;  //!< Normal event
  static const uint8_t CONFIRMATION_REQUEST =
      1;                                //!< Request to parent if event is caused by child or not.
  static const uint8_t YOUR_FAULT = 2;  //!< The fault was caused by child, parent believes it's ok.
  static const uint8_t MY_FAULT = 3;    //!< The fault was caused by the parent, child is ok.
  // Add other messageIDs here if necessary.
  static const uint8_t EVENT_MESSAGE_SIZE = HEADER_SIZE + sizeof(Event) + 3 * sizeof(uint32_t);

  EventMessage();
  //	EventMessage(EventId_t event, EventSeverity_t severity,
  //			object_id_t reporter, uint32_t parameter1, uint32_t parameter2 = 0);
  EventMessage(Event event, object_id_t reporter, uint32_t parameter1, uint32_t parameter2 = 0);
  virtual ~EventMessage();
  Event getEvent();
  void setEvent(Event event);
  uint8_t getMessageId();
  void setMessageId(uint8_t id);
  EventSeverity_t getSeverity();
  void setSeverity(EventSeverity_t severity);
  EventId_t getEventId();
  void setEventId(EventId_t event);
  object_id_t getReporter();
  void setReporter(object_id_t reporter);
  uint32_t getParameter1();
  void setParameter1(uint32_t parameter);
  uint32_t getParameter2();
  void setParameter2(uint32_t parameter);

  void clearEventMessage();
  bool isClearedEventMessage();

 protected:
  static const Event INVALID_EVENT = 0;
  virtual size_t getMinimumMessageSize() const override;
};

#endif /* FSFW_EVENTS_EVENTMESSAGE_H_ */
