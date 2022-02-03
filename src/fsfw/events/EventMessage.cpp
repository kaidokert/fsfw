#include "fsfw/events/EventMessage.h"

#include <cstring>

EventMessage::EventMessage() {
  messageSize = EVENT_MESSAGE_SIZE;
  clearEventMessage();
}

EventMessage::EventMessage(Event event, object_id_t reporter, uint32_t parameter1,
                           uint32_t parameter2) {
  messageSize = EVENT_MESSAGE_SIZE;
  setMessageId(EVENT_MESSAGE);
  setEvent(event);
  setReporter(reporter);
  setParameter1(parameter1);
  setParameter2(parameter2);
}

EventMessage::~EventMessage() {}

Event EventMessage::getEvent() {
  Event event;
  memcpy(&event, getData(), sizeof(Event));
  return (event & 0xFFFFFF);
}

void EventMessage::setEvent(Event event) {
  Event tempEvent;
  memcpy(&tempEvent, getData(), sizeof(Event));
  tempEvent = (tempEvent & 0xFF000000) + (event & 0xFFFFFF);
  memcpy(getData(), &tempEvent, sizeof(Event));
}

uint8_t EventMessage::getMessageId() {
  Event event;
  memcpy(&event, getData(), sizeof(Event));
  return (event & 0xFF000000) >> 24;
}

void EventMessage::setMessageId(uint8_t id) {
  Event event;
  memcpy(&event, getData(), sizeof(Event));
  event = (event & 0x00FFFFFF) + (id << 24);
  memcpy(getData(), &event, sizeof(Event));
}

EventSeverity_t EventMessage::getSeverity() {
  Event event;
  memcpy(&event, getData(), sizeof(Event));
  return event::getSeverity(event);
}

void EventMessage::setSeverity(EventSeverity_t severity) {
  Event event;
  memcpy(&event, getData(), sizeof(Event));
  event = (event & 0xFF00FFFF) + (severity << 16);
  memcpy(getData(), &event, sizeof(Event));
}

EventId_t EventMessage::getEventId() {
  Event event;
  memcpy(&event, getData(), sizeof(Event));
  return event::getEventId(event);
}

void EventMessage::setEventId(EventId_t eventId) {
  Event event;
  memcpy(&event, getData(), sizeof(Event));
  event = (event & 0xFFFF0000) + eventId;
  memcpy(getData(), &event, sizeof(Event));
}

object_id_t EventMessage::getReporter() {
  object_id_t parameter;
  memcpy(&parameter, getData() + sizeof(Event), sizeof(object_id_t));
  return parameter;
}

void EventMessage::setReporter(object_id_t reporter) {
  memcpy(getData() + sizeof(Event), &reporter, sizeof(object_id_t));
}

uint32_t EventMessage::getParameter1() {
  uint32_t parameter;
  memcpy(&parameter, getData() + sizeof(Event) + sizeof(object_id_t), 4);
  return parameter;
}

void EventMessage::setParameter1(uint32_t parameter) {
  memcpy(getData() + sizeof(Event) + sizeof(object_id_t), &parameter, 4);
}

uint32_t EventMessage::getParameter2() {
  uint32_t parameter;
  memcpy(&parameter, getData() + sizeof(Event) + sizeof(object_id_t) + 4, 4);
  return parameter;
}

void EventMessage::setParameter2(uint32_t parameter) {
  memcpy(getData() + sizeof(Event) + sizeof(object_id_t) + 4, &parameter, 4);
}

void EventMessage::clearEventMessage() { setEvent(INVALID_EVENT); }

bool EventMessage::isClearedEventMessage() { return getEvent() == INVALID_EVENT; }

size_t EventMessage::getMinimumMessageSize() const { return EVENT_MESSAGE_SIZE; }
