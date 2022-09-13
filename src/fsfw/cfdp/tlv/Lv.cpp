#include "Lv.h"

cfdp::Lv::Lv(const uint8_t* value, size_t size) : value(value, size, true) {
  if (size > 0) {
    zeroLen = false;
  }
}

cfdp::Lv::Lv(const std::vector<uint8_t>& data) : value(data.data(), data.size(), true) {
  if (!data.empty()) {
    zeroLen = false;
  }
}

cfdp::Lv::Lv() : value(static_cast<uint8_t*>(nullptr), 0, true) {}

cfdp::Lv::Lv(const Lv& other)
    : value(other.value.getConstBuffer(), other.value.getSerializedSize() - 1, true) {
  if (other.value.getSerializedSize() - 1 > 0) {
    zeroLen = false;
  }
}

cfdp::Lv& cfdp::Lv::operator=(const Lv& other) {
  size_t otherSize = 0;
  auto* otherVal = const_cast<uint8_t*>(other.getValue(&otherSize));
  if (otherVal == nullptr or otherSize == 0) {
    this->zeroLen = true;
  }
  this->value.setConstBuffer(otherVal, otherSize);
  return *this;
}

ReturnValue_t cfdp::Lv::serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                  Endianness streamEndianness) const {
  if (maxSize < 1) {
    return BUFFER_TOO_SHORT;
  }
  if (buffer == nullptr or size == nullptr) {
    return returnvalue::FAILED;
  }
  if (zeroLen) {
    **buffer = 0;
    *size += 1;
    *buffer += 1;
    return returnvalue::OK;
  }
  return value.serialize(buffer, size, maxSize, streamEndianness);
}

size_t cfdp::Lv::getSerializedSize() const {
  if (zeroLen) {
    return 1;
  } else if (value.getConstBuffer() == nullptr) {
    return 0;
  }
  return value.getSerializedSize();
}

ReturnValue_t cfdp::Lv::deSerialize(const uint8_t** buffer, size_t* size,
                                    Endianness streamEndianness) {
  if (buffer == nullptr or size == nullptr) {
    return returnvalue::FAILED;
  }
  if (*size < 1) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  size_t lengthField = **buffer;
  if (lengthField == 0) {
    zeroLen = true;
    *buffer += 1;
    *size -= 1;
    return returnvalue::OK;
  } else if (*size < lengthField + 1) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  zeroLen = false;
  // Zero-copy implementation
  value.setConstBuffer(*buffer + 1, lengthField);
  *buffer += 1 + lengthField;
  *size -= 1 + lengthField;
  return returnvalue::OK;
}

const uint8_t* cfdp::Lv::getValue(size_t* size) const {
  if (size != nullptr) {
    // Length without length field
    *size = value.getSerializedSize() - 1;
  }
  return value.getConstBuffer();
}
