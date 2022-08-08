#include "MetadataInfo.h"

MetadataInfo::MetadataInfo(bool closureRequested, cfdp::ChecksumType checksumType,
                           cfdp::FileSize& fileSize, cfdp::Lv& sourceFileName,
                           cfdp::Lv& destFileName)
    : closureRequested(closureRequested),
      checksumType(checksumType),
      fileSize(fileSize),
      sourceFileName(sourceFileName),
      destFileName(destFileName) {}

void MetadataInfo::setOptionsArray(cfdp::Tlv** optionsArray_, const size_t* optionsLen_,
                                   const size_t* maxOptionsLen_) {
  this->optionsArray = optionsArray_;
  if (maxOptionsLen_ != nullptr) {
    this->maxOptionsLen = *maxOptionsLen_;
  }
  if (optionsLen_ != nullptr) {
    this->optionsLen = *optionsLen_;
  }
}

cfdp::ChecksumType MetadataInfo::getChecksumType() const { return checksumType; }

void MetadataInfo::setChecksumType(cfdp::ChecksumType checksumType_) {
  checksumType = checksumType_;
}

bool MetadataInfo::isClosureRequested() const { return closureRequested; }

void MetadataInfo::setClosureRequested(bool closureRequested_) {
  closureRequested = closureRequested_;
}

cfdp::Lv& MetadataInfo::getDestFileName() { return destFileName; }

cfdp::FileSize& MetadataInfo::getFileSize() { return fileSize; }

ReturnValue_t MetadataInfo::getOptions(cfdp::Tlv*** optionsArray_, size_t* optionsLen_,
                                       size_t* maxOptsLen) {
  if (optionsArray_ == nullptr or optionsArray == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  *optionsArray_ = optionsArray;
  if (optionsLen_ != nullptr) {
    *optionsLen_ = this->optionsLen;
  }
  if (maxOptsLen != nullptr) {
    *maxOptsLen = this->maxOptionsLen;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

bool MetadataInfo::hasOptions() const {
  if (optionsArray != nullptr and optionsLen > 0) {
    return true;
  }
  return false;
}

bool MetadataInfo::canHoldOptions() const {
  if (optionsArray != nullptr and maxOptionsLen > 0) {
    return true;
  }
  return false;
}

size_t MetadataInfo::getSerializedSize(bool fssLarge) {
  // 1 byte + minimal FSS 4 bytes
  size_t size = 5;
  if (fssLarge) {
    size += 4;
  }
  size += sourceFileName.getSerializedSize();
  size += destFileName.getSerializedSize();
  if (hasOptions()) {
    for (size_t idx = 0; idx < optionsLen; idx++) {
      size += optionsArray[idx]->getSerializedSize();
    }
  }
  return size;
}

void MetadataInfo::setDestFileName(cfdp::Lv& destFileName_) { this->destFileName = destFileName_; }

void MetadataInfo::setSourceFileName(cfdp::Lv& sourceFileName_) {
  this->sourceFileName = sourceFileName_;
}

size_t MetadataInfo::getMaxOptionsLen() const { return maxOptionsLen; }

void MetadataInfo::setMaxOptionsLen(size_t maxOptionsLen_) { this->maxOptionsLen = maxOptionsLen_; }

size_t MetadataInfo::getOptionsLen() const { return optionsLen; }

void MetadataInfo::setOptionsLen(size_t optionsLen_) { this->optionsLen = optionsLen_; }

cfdp::Lv& MetadataInfo::getSourceFileName() { return sourceFileName; }
