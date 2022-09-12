#ifndef FSFW_TESTS_CDSSHORTTIMESTAMPERMOCK_H
#define FSFW_TESTS_CDSSHORTTIMESTAMPERMOCK_H

#include <array>

#include "fsfw/timemanager/TimeReaderIF.h"
#include "fsfw/timemanager/TimeWriterIF.h"

class CdsShortTimestamperMock : public TimeWriterIF, public TimeReaderIF {
 public:
  unsigned int serializeCallCount = 0;
  unsigned int deserializeCallCount = 0;
  ReturnValue_t lastDeserializeResult = returnvalue::OK;
  ReturnValue_t lastSerializeResult = returnvalue::OK;
  unsigned int getSizeCallCount = 0;
  bool nextSerFails = false;
  ReturnValue_t serFailRetval = returnvalue::FAILED;
  bool nextDeserFails = false;
  ReturnValue_t deserFailRetval = returnvalue::FAILED;
  std::array<uint8_t, 7> valueToStamp{};

  CdsShortTimestamperMock() = default;

  explicit CdsShortTimestamperMock(std::array<uint8_t, 7> valueToStamp)
      : valueToStamp(valueToStamp) {}

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override {
    auto &thisNonConst = const_cast<CdsShortTimestamperMock &>(*this);
    thisNonConst.serializeCallCount += 1;
    if (nextSerFails) {
      return serFailRetval;
    }
    if (*size + getSerializedSize() > maxSize) {
      thisNonConst.lastSerializeResult = SerializeIF::BUFFER_TOO_SHORT;
      return lastSerializeResult;
    }
    std::copy(valueToStamp.begin(), valueToStamp.end(), *buffer);
    thisNonConst.lastSerializeResult = returnvalue::OK;
    *buffer += getSerializedSize();
    *size += getSerializedSize();
    return lastSerializeResult;
  }
  [[nodiscard]] size_t getSerializedSize() const override {
    auto &thisNonConst = const_cast<CdsShortTimestamperMock &>(*this);
    thisNonConst.getSizeCallCount += 1;
    return valueToStamp.size();
  }
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override {
    deserializeCallCount += 1;
    if (nextDeserFails) {
      return deserFailRetval;
    }
    if (*size < 7) {
      lastDeserializeResult = SerializeIF::STREAM_TOO_SHORT;
      return lastDeserializeResult;
    }
    std::copy(*buffer, *buffer + getSerializedSize(), valueToStamp.begin());
    return lastDeserializeResult;
  }

  [[nodiscard]] size_t getTimestampSize() const override { return getSerializedSize(); }

  void reset() {
    serializeCallCount = 0;
    getSizeCallCount = 0;
    deserializeCallCount = 0;
    nextSerFails = false;
    nextDeserFails = false;
    lastSerializeResult = returnvalue::OK;
    lastDeserializeResult = returnvalue::OK;
    deserFailRetval = returnvalue::FAILED;
    serFailRetval = returnvalue::FAILED;
  }

  timeval &getTime() override { return dummyTime; }

 private:
  timeval dummyTime{};
};
#endif  // FSFW_TESTS_CDSSHORTTIMESTAMPERMOCK_H
