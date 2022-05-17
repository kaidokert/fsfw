#ifndef FRAMEWORK_EVENTS_EVENTMATCHING_EVENTRANGEMATCHERBASE_H_
#define FRAMEWORK_EVENTS_EVENTMATCHING_EVENTRANGEMATCHERBASE_H_

#include "../../events/EventMessage.h"
#include "../../globalfunctions/matching/RangeMatcher.h"
#include "../../globalfunctions/matching/SerializeableMatcherIF.h"

template <typename T>
class EventRangeMatcherBase : public SerializeableMatcherIF<EventMessage*> {
  friend class EventMatchTree;

 public:
  EventRangeMatcherBase(T from, T till, bool inverted) : rangeMatcher(from, till, inverted) {}
  virtual ~EventRangeMatcherBase() {}
  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const {
    return rangeMatcher.serialize(buffer, size, maxSize, streamEndianness);
  }
  size_t getSerializedSize() const { return rangeMatcher.getSerializedSize(); }
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size, Endianness streamEndianness) {
    return rangeMatcher.deSerialize(buffer, size, streamEndianness);
  }

 protected:
  RangeMatcher<T> rangeMatcher;
};

#endif /* FRAMEWORK_EVENTS_EVENTMATCHING_EVENTRANGEMATCHERBASE_H_ */
