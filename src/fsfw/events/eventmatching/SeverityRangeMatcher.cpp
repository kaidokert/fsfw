#include "fsfw/events/eventmatching/SeverityRangeMatcher.h"

#include "fsfw/events/EventMessage.h"
#include "fsfw/serialize/SerializeAdapter.h"

SeverityRangeMatcher::SeverityRangeMatcher(EventSeverity_t from, EventSeverity_t till,
                                           bool inverted)
    : EventRangeMatcherBase<EventSeverity_t>(from, till, inverted) {}

SeverityRangeMatcher::~SeverityRangeMatcher() {}

bool SeverityRangeMatcher::match(EventMessage* message) {
  return rangeMatcher.match(message->getSeverity());
}
