/*
 * ReporterRangeMatcher.h
 *
 *  Created on: 27.08.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_EVENTS_EVENTMATCHING_REPORTERRANGEMATCHER_H_
#define FRAMEWORK_EVENTS_EVENTMATCHING_REPORTERRANGEMATCHER_H_

#include <framework/events/eventmatching/EventRangeMatcherBase.h>

class ReporterRangeMatcher: public EventRangeMatcherBase<object_id_t> {
public:
	ReporterRangeMatcher(object_id_t lower, object_id_t upper, bool inverted);
	~ReporterRangeMatcher();
	bool match(EventMessage* message);
};

#endif /* FRAMEWORK_EVENTS_EVENTMATCHING_REPORTERRANGEMATCHER_H_ */
