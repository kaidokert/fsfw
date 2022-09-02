#include "FileDataInfo.h"

FileDataInfo::FileDataInfo(cfdp::FileSize &offset, const uint8_t *fileData, size_t fileSize)
    : offset(offset), fileData(fileData), fileSize(fileSize) {}

FileDataInfo::FileDataInfo(cfdp::FileSize &offset) : offset(offset) {}

void FileDataInfo::setSegmentMetadataFlag(bool enable) {
  if (enable) {
    segmentMetadataFlag = cfdp::SegmentMetadataFlag::PRESENT;
  } else {
    segmentMetadataFlag = cfdp::SegmentMetadataFlag::NOT_PRESENT;
  }
}

size_t FileDataInfo::getSerializedSize(bool largeFile) const {
  size_t sz = 0;
  if (segmentMetadataFlag == cfdp::SegmentMetadataFlag::PRESENT) {
    sz += 1 + segmentMetadataLen;
  }
  if (largeFile) {
    sz += 8;
  } else {
    sz += 4;
  }
  sz += fileSize;
  return sz;
}

cfdp::SegmentMetadataFlag FileDataInfo::getSegmentMetadataFlag() const {
  return this->segmentMetadataFlag;
}

bool FileDataInfo::hasSegmentMetadata() const {
  if (segmentMetadataFlag == cfdp::SegmentMetadataFlag::PRESENT) {
    return true;
  }
  return false;
}

cfdp::RecordContinuationState FileDataInfo::getRecordContinuationState() const {
  return this->recContState;
}

size_t FileDataInfo::getSegmentMetadataLen() const { return segmentMetadataLen; }

ReturnValue_t FileDataInfo::addSegmentMetadataInfo(cfdp::RecordContinuationState recContState,
                                                   const uint8_t *segmentMetadata,
                                                   size_t segmentMetadataLen) {
  this->segmentMetadataFlag = cfdp::SegmentMetadataFlag::PRESENT;
  this->recContState = recContState;
  if (segmentMetadataLen > 63) {
    return returnvalue::FAILED;
  }
  this->segmentMetadata = segmentMetadata;
  this->segmentMetadataLen = segmentMetadataLen;
  return returnvalue::OK;
}

const uint8_t *FileDataInfo::getFileData(size_t *fileSize_) const {
  if (fileSize_ != nullptr) {
    *fileSize_ = this->fileSize;
  }
  return fileData;
}

const uint8_t *FileDataInfo::getSegmentMetadata(size_t *segmentMetadataLen_) {
  if (segmentMetadataLen_ != nullptr) {
    *segmentMetadataLen_ = this->segmentMetadataLen;
  }
  return segmentMetadata;
}

cfdp::FileSize &FileDataInfo::getOffset() { return offset; }

void FileDataInfo::setRecordContinuationState(cfdp::RecordContinuationState recContState) {
  this->recContState = recContState;
}

void FileDataInfo::setSegmentMetadataLen(size_t len) { this->segmentMetadataLen = len; }

void FileDataInfo::setSegmentMetadata(const uint8_t *ptr) { this->segmentMetadata = ptr; }

void FileDataInfo::setFileData(const uint8_t *fileData, size_t fileSize) {
  this->fileData = fileData;
  this->fileSize = fileSize;
}

cfdp::SegmentationControl FileDataInfo::getSegmentationControl() const { return segCtrl; }

void FileDataInfo::setSegmentationControl(cfdp::SegmentationControl segCtrl) {
  this->segCtrl = segCtrl;
}
