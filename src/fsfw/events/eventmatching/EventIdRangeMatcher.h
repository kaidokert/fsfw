#ifndef FRAMEWORK_EVENTS_EVENTMATCHING_EVENTIDRANGEMATCHER_H_
#define FRAMEWORK_EVENTS_EVENTMATCHING_EVENTIDRANGEMATCHER_H_

#include "EventRangeMatcherBase.h"

class EventIdRangeMatcher : public EventRangeMatcherBase<EventId_t> {
 public:
  EventIdRangeMatcher(EventId_t lower, EventId_t upper, bool inverted);
  ~EventIdRangeMatcher();
  bool match(EventMessage* message);
};

#endif /* FRAMEWORK_EVENTS_EVENTMATCHING_EVENTIDRANGEMATCHER_H_ */
