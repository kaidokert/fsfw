#ifndef EVENTOBJECT_EVENT_H_
#define EVENTOBJECT_EVENT_H_

#include <stdint.h>
#include "fwSubsystemIdRanges.h"
// could be moved to more suitable location
#include <events/subsystemIdRanges.h>

typedef uint16_t EventId_t;
typedef uint8_t EventSeverity_t;

#define MAKE_EVENT(id, severity) (((severity)<<16)+(SUBSYSTEM_ID*100)+(id))

typedef uint32_t Event;

namespace event {

constexpr EventId_t getEventId(Event event) {
	return (event & 0xFFFF);
}

constexpr EventSeverity_t getSeverity(Event event) {
	return ((event >> 16) & 0xFF);
}

constexpr Event makeEvent(uint8_t subsystemId, uint8_t uniqueEventId,
		EventSeverity_t eventSeverity) {
	return (eventSeverity << 16) + (subsystemId * 100) + uniqueEventId;
}

}

namespace severity {
	static constexpr EventSeverity_t INFO = 1;
	static constexpr EventSeverity_t LOW = 2;
	static constexpr EventSeverity_t MEDIUM = 3;
	static constexpr EventSeverity_t HIGH = 4;
}

#endif /* EVENTOBJECT_EVENT_H_ */
