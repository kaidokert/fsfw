#ifndef FRAMEWORK_EVENTS_EVENTMATCHING_EVENTRANGEMATCHERBASE_H_
#define FRAMEWORK_EVENTS_EVENTMATCHING_EVENTRANGEMATCHERBASE_H_

#include <framework/events/EventMessage.h>
#include <framework/globalfunctions/matching/RangeMatcher.h>
#include <framework/globalfunctions/matching/SerializeableMatcherIF.h>

template <typename T>
class EventRangeMatcherBase: public SerializeableMatcherIF<EventMessage*> {
	friend class EventMatchTree;
public:
	EventRangeMatcherBase(T from, T till, bool inverted) : rangeMatcher(from, till, inverted) {	}
	virtual ~EventRangeMatcherBase() { }
	ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			const size_t max_size, bool bigEndian) const {
		return rangeMatcher.serialize(buffer, size, max_size, bigEndian);
	}
	uint32_t getSerializedSize() const {
		return rangeMatcher.getSerializedSize();
	}
	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		return rangeMatcher.deSerialize(buffer, size, bigEndian);
	}
protected:
	RangeMatcher<T> rangeMatcher;
};

#endif /* FRAMEWORK_EVENTS_EVENTMATCHING_EVENTRANGEMATCHERBASE_H_ */
