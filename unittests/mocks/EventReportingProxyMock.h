#ifndef FSFW_TESTS_EVENTREPORTPROXYMOCK_H
#define FSFW_TESTS_EVENTREPORTPROXYMOCK_H

#include <queue>

#include "fsfw/events/EventReportingProxyIF.h"

class EventReportingProxyMock : public EventReportingProxyIF {
 public:
  void forwardEvent(Event event, uint32_t parameter1, uint32_t parameter2) const override;

  struct EventInfo {
    EventInfo(Event event, uint32_t p1, uint32_t p2) : event(event), p1(p1), p2(p2) {}

    Event event;
    uint32_t p1;
    uint32_t p2;
  };
  mutable std::queue<EventInfo> eventQueue;
};
#endif  // FSFW_TESTS_EVENTREPORTPROXYMOCK_H
