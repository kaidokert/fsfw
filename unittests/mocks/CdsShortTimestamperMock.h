#ifndef FSFW_TESTS_CDSSHORTTIMESTAMPERMOCK_H
#define FSFW_TESTS_CDSSHORTTIMESTAMPERMOCK_H

#include <array>

#include "fsfw/timemanager/TimeStamperIF.h"

class CdsShortTimestamperMock : public TimeStamperIF {
 public:
  unsigned int serializeCallCount = 0;
  ReturnValue_t lastSerializeResult = HasReturnvaluesIF::RETURN_OK;
  unsigned int getSizeCallCount = 0;
  std::array<uint8_t, 7> valueToStamp{};

  CdsShortTimestamperMock() = default;

  explicit CdsShortTimestamperMock(std::array<uint8_t, 7> valueToStamp)
      : valueToStamp(valueToStamp) {}

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override {
    auto &thisNonConst = const_cast<CdsShortTimestamperMock &>(*this);
    thisNonConst.serializeCallCount += 1;
    if (*size + getSerializedSize() > maxSize) {
      thisNonConst.lastSerializeResult = SerializeIF::BUFFER_TOO_SHORT;
      return lastSerializeResult;
    }
    std::copy(valueToStamp.begin(), valueToStamp.end(), *buffer);
    thisNonConst.lastSerializeResult = HasReturnvaluesIF::RETURN_OK;
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
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  [[nodiscard]] size_t getTimestampSize() const override { return getSerializedSize(); }
  ReturnValue_t addTimeStamp(uint8_t *buffer, uint8_t maxSize) override { return 0; }

  void reset() {
    serializeCallCount = 0;
    getSizeCallCount = 0;
  }

 private:
};
#endif  // FSFW_TESTS_CDSSHORTTIMESTAMPERMOCK_H
