#include "HeaderSerializer.h"

#include "HeaderDeserializer.h"

HeaderSerializer::HeaderSerializer(PduConfig &pduConf, cfdp::PduType pduType,
                                   size_t initPduDataFieldLen,
                                   cfdp::SegmentMetadataFlag segmentMetadataFlag,
                                   cfdp::SegmentationControl segCtrl)
    : pduType(pduType),
      segmentMetadataFlag(segmentMetadataFlag),
      segmentationCtrl(segCtrl),
      pduDataFieldLen(initPduDataFieldLen),
      pduConf(pduConf) {}

ReturnValue_t HeaderSerializer::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
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

size_t HeaderSerializer::getSerializedSize() const {
  size_t shit = pduConf.seqNum.getWidth() + pduConf.sourceId.getWidth() * 2 + 4;
  return shit;
}

ReturnValue_t HeaderSerializer::deSerialize(const uint8_t **buffer, size_t *size,
                                            Endianness streamEndianness) {
  // We could implement this, but I prefer dedicated classes
  return returnvalue::FAILED;
}

size_t HeaderSerializer::getWholePduSize() const {
  // Return size of header plus the PDU data field length
  return pduDataFieldLen + HeaderSerializer::getSerializedSize();
}

size_t HeaderSerializer::getPduDataFieldLen() const { return pduDataFieldLen; }

void HeaderSerializer::setPduDataFieldLen(size_t pduDataFieldLen) {
  this->pduDataFieldLen = pduDataFieldLen;
}

void HeaderSerializer::setPduType(cfdp::PduType pduType) { this->pduType = pduType; }

void HeaderSerializer::setSegmentMetadataFlag(cfdp::SegmentMetadataFlag segmentMetadataFlag) {
  this->segmentMetadataFlag = segmentMetadataFlag;
}

cfdp::PduType HeaderSerializer::getPduType() const { return pduType; }

cfdp::Direction HeaderSerializer::getDirection() const { return pduConf.direction; }

cfdp::TransmissionModes HeaderSerializer::getTransmissionMode() const { return pduConf.mode; }

bool HeaderSerializer::getCrcFlag() const { return pduConf.crcFlag; }

bool HeaderSerializer::getLargeFileFlag() const { return pduConf.largeFile; }

cfdp::SegmentationControl HeaderSerializer::getSegmentationControl() const {
  return segmentationCtrl;
}

cfdp::WidthInBytes HeaderSerializer::getLenEntityIds() const { return pduConf.sourceId.getWidth(); }

cfdp::WidthInBytes HeaderSerializer::getLenSeqNum() const { return pduConf.seqNum.getWidth(); }

cfdp::SegmentMetadataFlag HeaderSerializer::getSegmentMetadataFlag() const {
  return segmentMetadataFlag;
}

void HeaderSerializer::getSourceId(cfdp::EntityId &sourceId) const { sourceId = pduConf.sourceId; }

void HeaderSerializer::getDestId(cfdp::EntityId &destId) const { destId = pduConf.destId; }

void HeaderSerializer::setSegmentationControl(cfdp::SegmentationControl segmentationControl) {
  this->segmentationCtrl = segmentationControl;
}

void HeaderSerializer::getTransactionSeqNum(cfdp::TransactionSeqNum &seqNum) const {
  seqNum = pduConf.seqNum;
}

bool HeaderSerializer::hasSegmentMetadataFlag() const {
  if (this->segmentMetadataFlag == cfdp::SegmentMetadataFlag::PRESENT) {
    return true;
  }
  return false;
}
