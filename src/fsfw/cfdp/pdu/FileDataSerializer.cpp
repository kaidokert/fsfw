#include "FileDataSerializer.h"

#include <cstring>

FileDataSerializer::FileDataSerializer(PduConfig& conf, FileDataInfo& info)
    : HeaderSerializer(conf, cfdp::PduType::FILE_DATA, 0, info.getSegmentMetadataFlag()),
      info(info) {
  update();
}

void FileDataSerializer::update() {
  this->setSegmentMetadataFlag(info.getSegmentMetadataFlag());
  this->setSegmentationControl(info.getSegmentationControl());
  setPduDataFieldLen(info.getSerializedSize(this->getLargeFileFlag()));
}

ReturnValue_t FileDataSerializer::serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                            Endianness streamEndianness) const {
  ReturnValue_t result = HeaderSerializer::serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (*size + this->getSerializedSize() > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  const uint8_t* readOnlyPtr = nullptr;
  if (this->hasSegmentMetadataFlag()) {
    size_t segmentMetadataLen = info.getSegmentMetadataLen();
    **buffer = info.getRecordContinuationState() << 6 | segmentMetadataLen;
    *buffer += 1;
    *size += 1;
    readOnlyPtr = info.getSegmentMetadata();
    std::memcpy(*buffer, readOnlyPtr, segmentMetadataLen);
    *buffer += segmentMetadataLen;
    *size += segmentMetadataLen;
  }
  cfdp::FileSize& offset = info.getOffset();
  result = offset.serialize(this->getLargeFileFlag(), buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  size_t fileSize = 0;
  readOnlyPtr = info.getFileData(&fileSize);
  if (*size + fileSize > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  std::memcpy(*buffer, readOnlyPtr, fileSize);
  *buffer += fileSize;
  *size += fileSize;
  return returnvalue::OK;
}

size_t FileDataSerializer::getSerializedSize() const {
  return HeaderSerializer::getSerializedSize() + info.getSerializedSize(this->getLargeFileFlag());
}
