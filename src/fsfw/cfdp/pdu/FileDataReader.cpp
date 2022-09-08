#include "FileDataReader.h"

FileDataReader::FileDataReader(const uint8_t* pduBuf, size_t maxSize, FileDataInfo& info)
    : PduHeaderReader(pduBuf, maxSize), info(info) {}

ReturnValue_t FileDataReader::parseData() {
  ReturnValue_t result = PduHeaderReader::parseData();
  if (result != returnvalue::OK) {
    return result;
  }
  size_t currentIdx = PduHeaderReader::getHeaderSize();
  const uint8_t* buf = pointers.rawPtr + currentIdx;
  size_t remSize = PduHeaderReader::getWholePduSize() - currentIdx;
  if (remSize < 1) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  if (hasSegmentMetadataFlag()) {
    info.setSegmentMetadataFlag(true);
    info.setRecordContinuationState(static_cast<cfdp::RecordContinuationState>((*buf >> 6) & 0b11));
    size_t segmentMetadataLen = *buf & 0b00111111;
    info.setSegmentMetadataLen(segmentMetadataLen);
    if (remSize < segmentMetadataLen + 1) {
      return SerializeIF::STREAM_TOO_SHORT;
    }
    if (segmentMetadataLen > 0) {
      buf += 1;
      remSize -= 1;
      info.setSegmentMetadata(buf);
      buf += segmentMetadataLen;
      remSize -= segmentMetadataLen;
    }
  }
  result = info.getOffset().deSerialize(&buf, &remSize, this->getEndianness());
  if (result != returnvalue::OK) {
    return result;
  }
  if (remSize > 0) {
    info.setFileData(buf, remSize);
  }
  return returnvalue::OK;
}

SerializeIF::Endianness FileDataReader::getEndianness() const { return endianness; }

void FileDataReader::setEndianness(SerializeIF::Endianness endianness_) {
  endianness = endianness_;
}
