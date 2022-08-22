#ifndef FSFW_TESTS_SIMPLESERIALIZABLE_H
#define FSFW_TESTS_SIMPLESERIALIZABLE_H

#include "fsfw/osal/Endiness.h"
#include "fsfw/serialize.h"

class SimpleSerializable : public SerializeIF {
 public:
  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override {
    if (*size + getSerializedSize() > maxSize) {
      return SerializeIF::BUFFER_TOO_SHORT;
    }
    **buffer = someU8;
    *buffer += 1;
    *size += 1;
    return SerializeAdapter::serialize(&someU16, buffer, size, maxSize, streamEndianness);
  }

  [[nodiscard]] size_t getSerializedSize() const override { return 3; }
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override {
    if (*size < getSerializedSize()) {
      return SerializeIF::STREAM_TOO_SHORT;
    }
    someU8 = **buffer;
    *size -= 1;
    *buffer += 1;
    return SerializeAdapter::deSerialize(&someU16, buffer, size, streamEndianness);
  }

  [[nodiscard]] uint8_t getU8() const { return someU8; }
  [[nodiscard]] uint16_t getU16() const { return someU16; }

 private:
  uint8_t someU8 = 1;
  uint16_t someU16 = 0x0203;
};

#endif  // FSFW_TESTS_SIMPLESERIALIZABLE_H
