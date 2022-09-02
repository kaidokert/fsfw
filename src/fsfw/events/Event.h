#ifndef EVENTOBJECT_EVENT_H_
#define EVENTOBJECT_EVENT_H_

#include <stdint.h>

#include "fwSubsystemIdRanges.h"

using EventId_t = uint16_t;
using EventSeverity_t = uint8_t;
using UniqueEventId_t = uint8_t;

namespace severity {
enum Severity : EventSeverity_t { INFO = 1, LOW = 2, MEDIUM = 3, HIGH = 4 };

}  // namespace severity

#define MAKE_EVENT(id, severity) (((severity) << 16) + (SUBSYSTEM_ID * 100) + (id))

typedef uint32_t Event;

namespace event {

constexpr EventId_t getEventId(Event event) { return (event & 0xFFFF); }

constexpr EventSeverity_t getSeverity(Event event) { return ((event >> 16) & 0xFF); }

constexpr Event makeEvent(uint8_t subsystemId, UniqueEventId_t uniqueEventId,
                          EventSeverity_t eventSeverity) {
  return (eventSeverity << 16) + (subsystemId * 100) + uniqueEventId;
}

}  // namespace event

#endif /* EVENTOBJECT_EVENT_H_ */
