#include "HeaderCreator.h"

HeaderCreator::HeaderCreator(PduConfig &pduConf, cfdp::PduType pduType, size_t initPduDataFieldLen,
                             cfdp::SegmentMetadataFlag segmentMetadataFlag,
                             cfdp::SegmentationControl segCtrl)
    : pduType(pduType),
      segmentMetadataFlag(segmentMetadataFlag),
      segmentationCtrl(segCtrl),
      pduDataFieldLen(initPduDataFieldLen),
      pduConf(pduConf) {}

ReturnValue_t HeaderCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                       Endianness streamEndianness) const {
  if (buffer == nullptr or size == nullptr) {
    return returnvalue::FAILED;
  }
  if (maxSize < this->getSerializedSize()) {
    return BUFFER_TOO_SHORT;
  }
  **buffer = cfdp::VERSION_BITS | this->pduType << 4 | pduConf.direction << 3 | pduConf.mode << 2 |
             pduConf.crcFlag << 1 | pduConf.largeFile;
  *buffer += 1;
  **buffer = (pduDataFieldLen & 0xff00) >> 8;
  *buffer += 1;
  **buffer = pduDataFieldLen & 0x00ff;
  *buffer += 1;
  **buffer = segmentationCtrl << 7 | pduConf.sourceId.getWidth() << 4 | segmentMetadataFlag << 3 |
             pduConf.seqNum.getWidth();
  *buffer += 1;
  *size += 4;
  ReturnValue_t result = pduConf.sourceId.serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = pduConf.seqNum.serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = pduConf.destId.serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }

  return returnvalue::OK;
}

size_t HeaderCreator::getSerializedSize() const {
  return pduConf.seqNum.getWidth() + pduConf.sourceId.getWidth() * 2 + 4;
}

ReturnValue_t HeaderCreator::deSerialize(const uint8_t **buffer, size_t *size,
                                         Endianness streamEndianness) {
  // We could implement this, but I prefer dedicated classes
  return returnvalue::FAILED;
}

size_t HeaderCreator::getWholePduSize() const {
  // Return size of header plus the PDU data field length
  return pduDataFieldLen + HeaderCreator::getSerializedSize();
}

size_t HeaderCreator::getPduDataFieldLen() const { return pduDataFieldLen; }

void HeaderCreator::setPduDataFieldLen(size_t pduDataFieldLen_) {
  pduDataFieldLen = pduDataFieldLen_;
}

void HeaderCreator::setPduType(cfdp::PduType pduType_) { pduType = pduType_; }

void HeaderCreator::setSegmentMetadataFlag(cfdp::SegmentMetadataFlag segmentMetadataFlag_) {
  segmentMetadataFlag = segmentMetadataFlag_;
}

cfdp::PduType HeaderCreator::getPduType() const { return pduType; }

cfdp::Direction HeaderCreator::getDirection() const { return pduConf.direction; }

cfdp::TransmissionMode HeaderCreator::getTransmissionMode() const { return pduConf.mode; }

bool HeaderCreator::getCrcFlag() const { return pduConf.crcFlag; }

bool HeaderCreator::getLargeFileFlag() const { return pduConf.largeFile; }

cfdp::SegmentationControl HeaderCreator::getSegmentationControl() const { return segmentationCtrl; }

cfdp::WidthInBytes HeaderCreator::getLenEntityIds() const { return pduConf.sourceId.getWidth(); }

cfdp::WidthInBytes HeaderCreator::getLenSeqNum() const { return pduConf.seqNum.getWidth(); }

cfdp::SegmentMetadataFlag HeaderCreator::getSegmentMetadataFlag() const {
  return segmentMetadataFlag;
}

void HeaderCreator::getSourceId(cfdp::EntityId &sourceId) const { sourceId = pduConf.sourceId; }

void HeaderCreator::getDestId(cfdp::EntityId &destId) const { destId = pduConf.destId; }

void HeaderCreator::setSegmentationControl(cfdp::SegmentationControl segmentationControl) {
  this->segmentationCtrl = segmentationControl;
}

void HeaderCreator::getTransactionSeqNum(cfdp::TransactionSeqNum &seqNum) const {
  seqNum = pduConf.seqNum;
}

bool HeaderCreator::hasSegmentMetadataFlag() const {
  if (this->segmentMetadataFlag == cfdp::SegmentMetadataFlag::PRESENT) {
    return true;
  }
  return false;
}
