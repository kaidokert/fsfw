/*
 * SeverityRangeMatcher.cpp
 *
 *  Created on: 27.08.2015
 *      Author: baetz
 */

#include <framework/events/eventmatching/SeverityRangeMatcher.h>
#include <framework/events/EventMessage.h>
#include <framework/serialize/SerializeAdapter.h>

SeverityRangeMatcher::SeverityRangeMatcher(EventSeverity_t from,
		EventSeverity_t till, bool inverted) : EventRangeMatcherBase<EventSeverity_t>(from, till, inverted) {
}

SeverityRangeMatcher::~SeverityRangeMatcher() {
}

bool SeverityRangeMatcher::match(EventMessage* message) {
	return rangeMatcher.match(message->getSeverity());
}
