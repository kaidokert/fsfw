#ifndef EVENTOBJECT_EVENT_H_
#define EVENTOBJECT_EVENT_H_

#include <stdint.h>
#include "fwSubsystemIdRanges.h"
//could be move to more suitable location
#include <events/subsystemIdRanges.h>

typedef uint16_t EventId_t;
typedef uint8_t EventSeverity_t;

#define MAKE_EVENT(id, severity) (((severity)<<16)+(SUBSYSTEM_ID*100)+(id))

typedef uint32_t Event;

namespace EVENT {
EventId_t getEventId(Event event);

EventSeverity_t getSeverity(Event event);

Event makeEvent(EventId_t eventId, EventSeverity_t eventSeverity);

}
namespace SEVERITY {
	static const EventSeverity_t INFO = 1;
	static const EventSeverity_t LOW = 2;
	static const EventSeverity_t MEDIUM = 3;
	static const EventSeverity_t HIGH = 4;
}

//Unfortunately, this does not work nicely because of the inability to define static classes in headers.
//struct Event {
//	Event(uint8_t domain, uint8_t counter, EventSeverity_t severity) :
//			id(domain*100+counter), severity(severity) {
//	}
//	EventId_t id;
//	EventSeverity_t severity;
//	static const EventSeverity_t INFO = 1;
//	static const EventSeverity_t LOW = 2;
//	static const EventSeverity_t MEDIUM = 3;
//	static const EventSeverity_t HIGH = 4;
//};

#endif /* EVENTOBJECT_EVENT_H_ */
