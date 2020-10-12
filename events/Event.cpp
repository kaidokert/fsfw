#include "Event.h"

namespace EVENT {
EventId_t getEventId(Event event) {
	return (event & 0xFFFF);
}

EventSeverity_t getSeverity(Event event) {
	return ((event >> 16) & 0xFF);
}

Event makeEvent(uint8_t subsystemId, uint8_t uniqueEventId,
		EventSeverity_t eventSeverity) {
	return (eventSeverity << 16) + (subsystemId * 100) + uniqueEventId;
}
}
