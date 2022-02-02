#include "EofInfo.h"

EofInfo::EofInfo(cfdp::ConditionCode conditionCode, uint32_t checksum, cfdp::FileSize fileSize,
                 EntityIdTlv* faultLoc)
    : conditionCode(conditionCode), checksum(checksum), fileSize(fileSize), faultLoc(faultLoc) {}

EofInfo::EofInfo(EntityIdTlv* faultLoc)
    : conditionCode(cfdp::ConditionCode::NO_CONDITION_FIELD),
      checksum(0),
      fileSize(0),
      faultLoc(faultLoc) {}

uint32_t EofInfo::getChecksum() const { return checksum; }

cfdp::ConditionCode EofInfo::getConditionCode() const { return conditionCode; }

EntityIdTlv* EofInfo::getFaultLoc() const { return faultLoc; }

cfdp::FileSize& EofInfo::getFileSize() { return fileSize; }

void EofInfo::setChecksum(uint32_t checksum) { this->checksum = checksum; }

void EofInfo::setConditionCode(cfdp::ConditionCode conditionCode) {
  this->conditionCode = conditionCode;
}

void EofInfo::setFaultLoc(EntityIdTlv* faultLoc) { this->faultLoc = faultLoc; }

size_t EofInfo::getSerializedSize(bool fssLarge) {
  // Condition code + spare + 4 byte checksum
  size_t size = 5;
  if (fssLarge) {
    size += 8;
  } else {
    size += 4;
  }
  // Do not account for fault location if the condition code is NO_ERROR. We assume that
  // a serializer will not serialize the fault location here.
  if (getFaultLoc() != nullptr and getConditionCode() != cfdp::ConditionCode::NO_ERROR) {
    size += faultLoc->getSerializedSize();
  }
  return size;
}

ReturnValue_t EofInfo::setFileSize(size_t fileSize, bool isLarge) {
  return this->fileSize.setFileSize(fileSize, isLarge);
}
