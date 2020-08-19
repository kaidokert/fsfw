#include "SeverityRangeMatcher.h"
#include "../../events/EventMessage.h"
#include "../../serialize/SerializeAdapter.h"

SeverityRangeMatcher::SeverityRangeMatcher(EventSeverity_t from,
		EventSeverity_t till, bool inverted) : EventRangeMatcherBase<EventSeverity_t>(from, till, inverted) {
}

SeverityRangeMatcher::~SeverityRangeMatcher() {
}

bool SeverityRangeMatcher::match(EventMessage* message) {
	return rangeMatcher.match(message->getSeverity());
}
