#include "fsfw/events/eventmatching/EventIdRangeMatcher.h"

EventIdRangeMatcher::EventIdRangeMatcher(EventId_t lower, EventId_t upper, bool inverted)
    : EventRangeMatcherBase<EventId_t>(lower, upper, inverted) {}

EventIdRangeMatcher::~EventIdRangeMatcher() {}

bool EventIdRangeMatcher::match(EventMessage* message) {
  return rangeMatcher.match(message->getEventId());
}
