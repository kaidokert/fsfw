#include "HeaderReader.h"

#include <fsfw/serialize/SerializeAdapter.h>

#include <cstring>

HeaderReader::HeaderReader(const uint8_t *pduBuf, size_t maxSize) { setData(pduBuf, maxSize); }

ReturnValue_t HeaderReader::parseData() {
  if (pointers.rawPtr == nullptr) {
    return returnvalue::FAILED;
  }
  if (maxSize < 7) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  pointers.fixedHeader =
      reinterpret_cast<PduHeaderFixedStruct *>(const_cast<uint8_t *>(pointers.rawPtr));
  sourceIdRaw = static_cast<uint8_t *>(&pointers.fixedHeader->variableFieldsStart);
  cfdp::WidthInBytes widthEntityIds = getLenEntityIds();
  cfdp::WidthInBytes widthSeqNum = getLenSeqNum();
  seqNumRaw = static_cast<uint8_t *>(sourceIdRaw) + static_cast<uint8_t>(widthEntityIds);
  destIdRaw = static_cast<uint8_t *>(seqNumRaw) + static_cast<uint8_t>(widthSeqNum);
  return returnvalue::OK;
}

ReturnValue_t HeaderReader::setData(uint8_t *dataPtr, size_t maxSize_, void *args) {
  if (dataPtr == nullptr) {
    return returnvalue::FAILED;
  }
  if (maxSize_ < 7) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  pointers.rawPtr = dataPtr;
  maxSize = maxSize_;
  return returnvalue::OK;
}

size_t HeaderReader::getHeaderSize() const {
  if (pointers.fixedHeader != nullptr) {
    return getLenEntityIds() * 2 + getLenSeqNum() + 4;
  }
  return 0;
}

size_t HeaderReader::getPduDataFieldLen() const {
  return (pointers.fixedHeader->pduDataFieldLenH << 8) | pointers.fixedHeader->pduDataFieldLenL;
}

size_t HeaderReader::getWholePduSize() const {
  return getPduDataFieldLen() + HeaderReader::getHeaderSize();
}

cfdp::PduType HeaderReader::getPduType() const {
  return static_cast<cfdp::PduType>((pointers.fixedHeader->firstByte >> 4) & 0x01);
}

cfdp::Direction HeaderReader::getDirection() const {
  return static_cast<cfdp::Direction>((pointers.fixedHeader->firstByte >> 3) & 0x01);
}

cfdp::TransmissionModes HeaderReader::getTransmissionMode() const {
  return static_cast<cfdp::TransmissionModes>((pointers.fixedHeader->firstByte >> 2) & 0x01);
}

bool HeaderReader::getCrcFlag() const { return (pointers.fixedHeader->firstByte >> 1) & 0x01; }

bool HeaderReader::getLargeFileFlag() const { return pointers.fixedHeader->firstByte & 0x01; }

cfdp::SegmentationControl HeaderReader::getSegmentationControl() const {
  return static_cast<cfdp::SegmentationControl>((pointers.fixedHeader->fourthByte >> 7) & 0x01);
}

cfdp::WidthInBytes HeaderReader::getLenEntityIds() const {
  return static_cast<cfdp::WidthInBytes>((pointers.fixedHeader->fourthByte >> 4) & 0x07);
}

cfdp::WidthInBytes HeaderReader::getLenSeqNum() const {
  return static_cast<cfdp::WidthInBytes>(pointers.fixedHeader->fourthByte & 0x07);
}

cfdp::SegmentMetadataFlag HeaderReader::getSegmentMetadataFlag() const {
  return static_cast<cfdp::SegmentMetadataFlag>((pointers.fixedHeader->fourthByte >> 3) & 0x01);
}

void HeaderReader::getSourceId(cfdp::EntityId &sourceId) const {
  assignVarLenField(dynamic_cast<cfdp::VarLenField *>(&sourceId), getLenEntityIds(),
                    this->sourceIdRaw);
}

void HeaderReader::getDestId(cfdp::EntityId &destId) const {
  assignVarLenField(dynamic_cast<cfdp::VarLenField *>(&destId), getLenEntityIds(), this->destIdRaw);
}

void HeaderReader::getTransactionSeqNum(cfdp::TransactionSeqNum &seqNum) const {
  assignVarLenField(dynamic_cast<cfdp::VarLenField *>(&seqNum), getLenSeqNum(), this->seqNumRaw);
}

void HeaderReader::assignVarLenField(cfdp::VarLenField *field, cfdp::WidthInBytes width,
                                     void *sourcePtr) const {
  switch (width) {
    case (cfdp::WidthInBytes::ONE_BYTE): {
      auto *fieldTyped = static_cast<uint8_t *>(sourcePtr);
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

size_t HeaderReader::getMaxSize() const { return maxSize; }

bool HeaderReader::hasSegmentMetadataFlag() const {
  if (this->getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::PRESENT) {
    return true;
  }
  return false;
}

ReturnValue_t HeaderReader::setData(const uint8_t *dataPtr, size_t maxSize_) {
  return setData(const_cast<uint8_t *>(dataPtr), maxSize_, nullptr);
}
bool HeaderReader::isNull() const {
  return pointers.rawPtr == nullptr or pointers.fixedHeader == nullptr;
}

HeaderReader::operator bool() const { return not isNull(); }

void HeaderReader::fillConfig(PduConfig &cfg) const {
  cfg.largeFile = getLargeFileFlag();
  cfg.crcFlag = getCrcFlag();
  cfg.mode = getTransmissionMode();
  cfg.direction = getDirection();
  getTransactionSeqNum(cfg.seqNum);
  getSourceId(cfg.sourceId);
  getDestId(cfg.destId);
}
