/*
 * SeverityRangeMatcher.h
 *
 *  Created on: 27.08.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_EVENTS_EVENTMATCHING_SEVERITYRANGEMATCHER_H_
#define FRAMEWORK_EVENTS_EVENTMATCHING_SEVERITYRANGEMATCHER_H_

#include <framework/events/eventmatching/EventRangeMatcherBase.h>

class SeverityRangeMatcher: public EventRangeMatcherBase<EventSeverity_t> {
public:
	SeverityRangeMatcher(EventSeverity_t from, EventSeverity_t till, bool inverted);
	~SeverityRangeMatcher();
	bool match(EventMessage* message);
};

#endif /* FRAMEWORK_EVENTS_EVENTMATCHING_SEVERITYRANGEMATCHER_H_ */
