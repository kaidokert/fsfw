#include "fsfw/events/eventmatching/ReporterRangeMatcher.h"

ReporterRangeMatcher::ReporterRangeMatcher(object_id_t lower, object_id_t upper, bool inverted)
    : EventRangeMatcherBase<object_id_t>(lower, upper, inverted) {}

ReporterRangeMatcher::~ReporterRangeMatcher() {}

bool ReporterRangeMatcher::match(EventMessage* message) {
  return rangeMatcher.match(message->getReporter());
}
