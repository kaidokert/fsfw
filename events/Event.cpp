/*
 * Event.cpp
 *
 *  Created on: 25.08.2015
 *      Author: baetz
 */




#include <framework/events/Event.h>
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
