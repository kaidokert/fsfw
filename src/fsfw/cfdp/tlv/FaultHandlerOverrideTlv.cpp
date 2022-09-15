#include "FaultHandlerOverrideTlv.h"

FaultHandlerOverrideTlv::FaultHandlerOverrideTlv(cfdp::ConditionCode conditionCode,
                                                 cfdp::FaultHandlerCode handlerCode)
    : conditionCode(conditionCode), handlerCode(handlerCode) {}

FaultHandlerOverrideTlv::FaultHandlerOverrideTlv() = default;

uint8_t FaultHandlerOverrideTlv::getLengthField() const { return 1; }

ReturnValue_t FaultHandlerOverrideTlv::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                                 Endianness streamEndianness) const {
  if ((maxSize < 3) or ((*size + 3) > maxSize)) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  **buffer = getType();
  *size += 1;
  *buffer += 1;
  **buffer = getLengthField();
  *size += 1;
  *buffer += 1;
  **buffer = this->conditionCode << 4 | this->handlerCode;
  *buffer += 1;
  *size += 1;
  return returnvalue::OK;
}

size_t FaultHandlerOverrideTlv::getSerializedSize() const { return getLengthField() + 2; }

ReturnValue_t FaultHandlerOverrideTlv::deSerialize(const uint8_t **buffer, size_t *size,
                                                   Endianness streamEndianness) {
  if (*size < 3) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  auto detectedType = static_cast<cfdp::TlvType>(**buffer);
  if (detectedType != cfdp::TlvType::FAULT_HANDLER) {
    return cfdp::INVALID_TLV_TYPE;
  }
  *buffer += 1;
  *size -= 1;
  size_t detectedSize = **buffer;
  if (detectedSize != getLengthField()) {
    return returnvalue::FAILED;
  }
  *buffer += 1;
  *size += 1;
  this->conditionCode = static_cast<cfdp::ConditionCode>((**buffer >> 4) & 0x0f);
  this->handlerCode = static_cast<cfdp::FaultHandlerCode>(**buffer & 0x0f);
  *buffer += 1;
  *size += 1;
  return returnvalue::OK;
}

cfdp::TlvType FaultHandlerOverrideTlv::getType() const { return cfdp::TlvType::FAULT_HANDLER; }
