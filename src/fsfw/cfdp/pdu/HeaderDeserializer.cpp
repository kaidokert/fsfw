#include "HeaderDeserializer.h"

#include <fsfw/serialize/SerializeAdapter.h>

#include <cstring>

HeaderDeserializer::HeaderDeserializer(const uint8_t *pduBuf, size_t maxSize)
    : rawPtr(pduBuf), maxSize(maxSize) {}

ReturnValue_t HeaderDeserializer::parseData() {
  if (maxSize < 7) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  return setData(const_cast<uint8_t *>(rawPtr), maxSize);
}

ReturnValue_t HeaderDeserializer::setData(uint8_t *dataPtr, size_t maxSize, void *args) {
  if (dataPtr == nullptr) {
    // Allowed for now
    this->fixedHeader = nullptr;
    return returnvalue::OK;
  }
  this->fixedHeader = reinterpret_cast<PduHeaderFixedStruct *>(const_cast<uint8_t *>(dataPtr));
  sourceIdRaw = static_cast<uint8_t *>(&fixedHeader->variableFieldsStart);
  cfdp::WidthInBytes widthEntityIds = getLenEntityIds();
  cfdp::WidthInBytes widthSeqNum = getLenSeqNum();
  seqNumRaw = static_cast<uint8_t *>(sourceIdRaw) + static_cast<uint8_t>(widthEntityIds);
  destIdRaw = static_cast<uint8_t *>(seqNumRaw) + static_cast<uint8_t>(widthSeqNum);
  this->maxSize = maxSize;
  return returnvalue::OK;
}

size_t HeaderDeserializer::getHeaderSize() const {
  if (fixedHeader != nullptr) {
    return getLenEntityIds() * 2 + getLenSeqNum() + 4;
  }
  return 0;
}

size_t HeaderDeserializer::getPduDataFieldLen() const {
  uint16_t pduFiedlLen = (fixedHeader->pduDataFieldLenH << 8) | fixedHeader->pduDataFieldLenL;
  return pduFiedlLen;
}

size_t HeaderDeserializer::getWholePduSize() const {
  return getPduDataFieldLen() + getHeaderSize();
}

cfdp::PduType HeaderDeserializer::getPduType() const {
  return static_cast<cfdp::PduType>((fixedHeader->firstByte >> 4) & 0x01);
}

cfdp::Direction HeaderDeserializer::getDirection() const {
  return static_cast<cfdp::Direction>((fixedHeader->firstByte >> 3) & 0x01);
}

cfdp::TransmissionModes HeaderDeserializer::getTransmissionMode() const {
  return static_cast<cfdp::TransmissionModes>((fixedHeader->firstByte >> 2) & 0x01);
}

bool HeaderDeserializer::getCrcFlag() const { return (fixedHeader->firstByte >> 1) & 0x01; }

bool HeaderDeserializer::getLargeFileFlag() const { return fixedHeader->firstByte & 0x01; }

cfdp::SegmentationControl HeaderDeserializer::getSegmentationControl() const {
  return static_cast<cfdp::SegmentationControl>((fixedHeader->fourthByte >> 7) & 0x01);
}

cfdp::WidthInBytes HeaderDeserializer::getLenEntityIds() const {
  return static_cast<cfdp::WidthInBytes>((fixedHeader->fourthByte >> 4) & 0x07);
}

cfdp::WidthInBytes HeaderDeserializer::getLenSeqNum() const {
  return static_cast<cfdp::WidthInBytes>(fixedHeader->fourthByte & 0x07);
}

cfdp::SegmentMetadataFlag HeaderDeserializer::getSegmentMetadataFlag() const {
  return static_cast<cfdp::SegmentMetadataFlag>((fixedHeader->fourthByte >> 3) & 0x01);
}

void HeaderDeserializer::getSourceId(cfdp::EntityId &sourceId) const {
  assignVarLenField(dynamic_cast<cfdp::VarLenField *>(&sourceId), getLenEntityIds(),
                    this->sourceIdRaw);
}

void HeaderDeserializer::getDestId(cfdp::EntityId &destId) const {
  assignVarLenField(dynamic_cast<cfdp::VarLenField *>(&destId), getLenEntityIds(), this->destIdRaw);
}

void HeaderDeserializer::getTransactionSeqNum(cfdp::TransactionSeqNum &seqNum) const {
  assignVarLenField(dynamic_cast<cfdp::VarLenField *>(&seqNum), getLenSeqNum(), this->seqNumRaw);
}

void HeaderDeserializer::assignVarLenField(cfdp::VarLenField *field, cfdp::WidthInBytes width,
                                           void *sourcePtr) const {
  switch (width) {
    case (cfdp::WidthInBytes::ONE_BYTE): {
      uint8_t *fieldTyped = static_cast<uint8_t *>(sourcePtr);
      field->setValue(width, *fieldTyped);
      break;
    }
    case (cfdp::WidthInBytes::TWO_BYTES): {
      uint16_t fieldTyped = 0;
      size_t deserSize = 0;
      SerializeAdapter::deSerialize(&fieldTyped, static_cast<uint8_t *>(sourcePtr), &deserSize,
                                    SerializeIF::Endianness::NETWORK);
      field->setValue(width, fieldTyped);
      break;
    }
    case (cfdp::WidthInBytes::FOUR_BYTES): {
      uint32_t fieldTyped = 0;
      size_t deserSize = 0;
      SerializeAdapter::deSerialize(&fieldTyped, static_cast<uint8_t *>(sourcePtr), &deserSize,
                                    SerializeIF::Endianness::NETWORK);
      field->setValue(width, fieldTyped);
      break;
    }
  }
}

size_t HeaderDeserializer::getMaxSize() const { return maxSize; }

bool HeaderDeserializer::hasSegmentMetadataFlag() const {
  if (this->getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::PRESENT) {
    return true;
  }
  return false;
}
