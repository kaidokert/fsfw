#ifndef FSFW_UTIL_UNSIGNEDBYTEFIELD_H
#define FSFW_UTIL_UNSIGNEDBYTEFIELD_H

#include "fsfw/serialize.h"

template <typename T>
class UnsignedByteField : public SerializeIF {
 public:
  static_assert(std::is_unsigned<T>::value);

  explicit UnsignedByteField(T value) : value(value) {}
  [[nodiscard]] ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                        Endianness streamEndianness) const override {
    return SerializeAdapter::serialize(&value, buffer, size, maxSize, streamEndianness);
  }

  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override {
    return SerializeAdapter::deSerialize(&value, buffer, size, streamEndianness);
  }

  [[nodiscard]] size_t getSerializedSize() const override { return sizeof(T); }

  [[nodiscard]] T getValue() const { return value; }

  void setValue(T value_) { value = value_; }

 private:
  T value;
};

class U32ByteField : public UnsignedByteField<uint32_t> {
 public:
  explicit U32ByteField(uint32_t value) : UnsignedByteField<uint32_t>(value) {}
};

class U16ByteField : public UnsignedByteField<uint16_t> {
 public:
  explicit U16ByteField(uint16_t value) : UnsignedByteField<uint16_t>(value) {}
};

class U8ByteField : public UnsignedByteField<uint8_t> {
 public:
  explicit U8ByteField(uint8_t value) : UnsignedByteField<uint8_t>(value) {}
};

#endif  // FSFW_UTIL_UNSIGNEDBYTEFIELD_H
