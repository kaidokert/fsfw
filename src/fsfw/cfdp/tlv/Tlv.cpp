#include "Tlv.h"

cfdp::Tlv::Tlv(TlvTypes type, const uint8_t *value, size_t size)
    : type(type), value(value, size, true) {
  if (size > 0) {
    zeroLen = false;
  }
}

cfdp::Tlv::Tlv() : value(static_cast<uint8_t *>(nullptr), 0, true) {}

ReturnValue_t cfdp::Tlv::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                   Endianness streamEndianness) const {
  if (buffer == nullptr or size == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  if (*size + 2 > maxSize) {
    return BUFFER_TOO_SHORT;
  }
  if (type == TlvTypes::INVALID_TLV) {
    return INVALID_TLV_TYPE;
  }
  **buffer = type;
  *size += 1;
  *buffer += 1;

  if (zeroLen) {
    **buffer = 0;
    *size += 1;
    *buffer += 1;
    return HasReturnvaluesIF::RETURN_OK;
  }
  if (value.getConstBuffer() == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  return value.serialize(buffer, size, maxSize, streamEndianness);
}

size_t cfdp::Tlv::getSerializedSize() const {
  if (zeroLen) {
    return 2;
  } else if (value.getConstBuffer() == nullptr) {
    return 0;
  }
  return value.getSerializedSize() + 1;
}

ReturnValue_t cfdp::Tlv::deSerialize(const uint8_t **buffer, size_t *size,
                                     Endianness streamEndianness) {
  if (buffer == nullptr or size == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  if (*size < 2) {
    return STREAM_TOO_SHORT;
  }

  uint8_t rawType = **buffer;
  if (not checkType(rawType)) {
    return INVALID_TLV_TYPE;
  }

  type = static_cast<TlvTypes>(rawType);
  *buffer += 1;
  *size -= 1;

  size_t lengthField = **buffer;
  if (lengthField == 0) {
    zeroLen = true;
    *buffer += 1;
    *size -= 1;
    return HasReturnvaluesIF::RETURN_OK;
  }
  if (lengthField + 1 > *size) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  zeroLen = false;
  // Zero-copy implementation
  value.setBuffer(const_cast<uint8_t *>(*buffer + 1), lengthField);
  *buffer += 1 + lengthField;
  *size -= 1 + lengthField;
  return HasReturnvaluesIF::RETURN_OK;
}

const uint8_t *cfdp::Tlv::getValue() const { return value.getConstBuffer(); }

cfdp::TlvTypes cfdp::Tlv::getType() const { return type; }

bool cfdp::Tlv::checkType(uint8_t rawType) {
  if (rawType != 0x03 and rawType <= 6) {
    return true;
  }
  return false;
}

void cfdp::Tlv::setValue(uint8_t *value, size_t len) {
  if (len > 0) {
    zeroLen = false;
  }
  this->value.setBuffer(value, len);
}

uint8_t cfdp::Tlv::getLengthField() const { return this->value.getSerializedSize() - 1; }

void cfdp::Tlv::setType(TlvTypes type) { this->type = type; }
