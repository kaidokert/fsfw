#include "EventReportingProxyMock.h"

void EventReportingProxyMock::forwardEvent(Event event, uint32_t parameter1,
                                           uint32_t parameter2) const {
  eventQueue.emplace(event, parameter1, parameter2);
}
