#include "EofInfo.h"

EofInfo::EofInfo(cfdp::ConditionCodes conditionCode, uint32_t checksum, cfdp::FileSize fileSize,
                 EntityIdTlv* faultLoc)
    : conditionCode(conditionCode), checksum(checksum), fileSize(fileSize), faultLoc(faultLoc) {}

EofInfo::EofInfo(EntityIdTlv* faultLoc)
    : conditionCode(cfdp::ConditionCodes::NO_CONDITION_FIELD),
      checksum(0),
      fileSize(0),
      faultLoc(faultLoc) {}

uint32_t EofInfo::getChecksum() const { return checksum; }

cfdp::ConditionCodes EofInfo::getConditionCode() const { return conditionCode; }

EntityIdTlv* EofInfo::getFaultLoc() const { return faultLoc; }

cfdp::FileSize& EofInfo::getFileSize() { return fileSize; }

void EofInfo::setChecksum(uint32_t checksum) { this->checksum = checksum; }

void EofInfo::setConditionCode(cfdp::ConditionCodes conditionCode) {
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
  if (getFaultLoc() != nullptr and getConditionCode() != cfdp::ConditionCodes::NO_ERROR) {
    size += faultLoc->getSerializedSize();
  }
  return size;
}

ReturnValue_t EofInfo::setFileSize(size_t fileSize, bool isLarge) {
  return this->fileSize.setFileSize(fileSize, isLarge);
}
