#ifndef RANGEMATCHER_H_
#define RANGEMATCHER_H_

#include "../../serialize/SerializeAdapter.h"
#include "SerializeableMatcherIF.h"

template <typename T>
class RangeMatcher : public SerializeableMatcherIF<T> {
 public:
  bool inverted;
  T lowerBound;
  T upperBound;

  RangeMatcher() : inverted(true), lowerBound(1), upperBound(0) {}
  RangeMatcher(T lowerBound, T upperBound, bool inverted = false)
      : inverted(inverted), lowerBound(lowerBound), upperBound(upperBound) {}

  bool match(T input) override {
    if (inverted) {
      return !doMatch(input);
    } else {
      return doMatch(input);
    }
  }

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          SerializeIF::Endianness streamEndianness) const override {
    ReturnValue_t result =
        SerializeAdapter::serialize(&lowerBound, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&upperBound, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    return SerializeAdapter::serialize(&inverted, buffer, size, maxSize, streamEndianness);
  }

  size_t getSerializedSize() const override {
    return sizeof(lowerBound) + sizeof(upperBound) + sizeof(bool);
  }

  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            SerializeIF::Endianness streamEndianness) override {
    ReturnValue_t result =
        SerializeAdapter::deSerialize(&lowerBound, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::deSerialize(&upperBound, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    return SerializeAdapter::deSerialize(&inverted, buffer, size, streamEndianness);
  }

 protected:
  bool doMatch(T input) { return (input >= lowerBound) && (input <= upperBound); }
};

#endif /* RANGEMATCHER_H_ */
