#include "Event.h"
namespace EVENT {
EventId_t getEventId(Event event) {
	return (event & 0xFFFF);
}

EventSeverity_t getSeverity(Event event) {
	return ((event >> 16) & 0xFF);
}

Event makeEvent(EventId_t eventId, EventSeverity_t eventSeverity) {
	return (eventSeverity << 16) + (eventId & 0xFFFF);
}
}
