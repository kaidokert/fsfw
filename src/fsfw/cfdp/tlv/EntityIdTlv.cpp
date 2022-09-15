#include "EntityIdTlv.h"

#include <array>

EntityIdTlv::EntityIdTlv(cfdp::EntityId &entityId) : entityId(entityId) {}

EntityIdTlv::~EntityIdTlv() {}

ReturnValue_t EntityIdTlv::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                     Endianness streamEndianness) const {
  if (maxSize < this->getSerializedSize()) {
    return BUFFER_TOO_SHORT;
  }
  **buffer = cfdp::TlvType::ENTITY_ID;
  *size += 1;
  *buffer += 1;
  size_t serLen = entityId.getSerializedSize();
  **buffer = serLen;
  *size += 1;
  *buffer += 1;
  return entityId.serialize(buffer, size, maxSize, streamEndianness);
}

size_t EntityIdTlv::getSerializedSize() const { return getLengthField() + 1; }

ReturnValue_t EntityIdTlv::deSerialize(const uint8_t **buffer, size_t *size,
                                       Endianness streamEndianness) {
  if (*size < 3) {
    return STREAM_TOO_SHORT;
  }
  cfdp::TlvType type = static_cast<cfdp::TlvType>(**buffer);
  if (type != cfdp::TlvType::ENTITY_ID) {
    return cfdp::INVALID_TLV_TYPE;
  }
  *buffer += 1;
  *size -= 1;

  cfdp::WidthInBytes width = static_cast<cfdp::WidthInBytes>(**buffer);
  if (*size < static_cast<size_t>(1 + width)) {
    return STREAM_TOO_SHORT;
  }
  *buffer += 1;
  *size -= 1;

  return entityId.deSerialize(width, buffer, size, streamEndianness);
}

ReturnValue_t EntityIdTlv::deSerialize(cfdp::Tlv &tlv, Endianness endianness) {
  const uint8_t *ptr = tlv.getValue() + 2;
  size_t remSz = tlv.getSerializedSize() - 2;
  cfdp::WidthInBytes width = static_cast<cfdp::WidthInBytes>(remSz);
  return entityId.deSerialize(width, &ptr, &remSz, endianness);
}

uint8_t EntityIdTlv::getLengthField() const { return 1 + entityId.getSerializedSize(); }

cfdp::TlvType EntityIdTlv::getType() const { return cfdp::TlvType::ENTITY_ID; }

cfdp::EntityId &EntityIdTlv::getEntityId() { return entityId; }
