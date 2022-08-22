#include "FinishedInfo.h"

FinishedInfo::FinishedInfo() {}

FinishedInfo::FinishedInfo(cfdp::ConditionCode conditionCode, cfdp::FileDeliveryCode deliveryCode,
                           cfdp::FileDeliveryStatus fileStatus)
    : conditionCode(conditionCode), deliveryCode(deliveryCode), fileStatus(fileStatus) {}

size_t FinishedInfo::getSerializedSize() const {
  size_t size = 1;
  if (hasFsResponses()) {
    for (size_t idx = 0; idx < fsResponsesLen; idx++) {
      size += fsResponses[idx]->getSerializedSize();
    }
  }
  if (this->faultLocation != nullptr) {
    size += faultLocation->getSerializedSize();
  }
  return size;
}

bool FinishedInfo::hasFsResponses() const {
  if (fsResponses != nullptr and fsResponsesLen > 0) {
    return true;
  }
  return false;
}
bool FinishedInfo::canHoldFsResponses() const {
  if (fsResponses != nullptr and fsResponsesMaxLen > 0) {
    return true;
  }
  return false;
}

ReturnValue_t FinishedInfo::setFilestoreResponsesArray(FilestoreResponseTlv** fsResponses,
                                                       size_t* fsResponsesLen,
                                                       const size_t* maxFsResponsesLen) {
  this->fsResponses = fsResponses;
  if (fsResponsesLen != nullptr) {
    this->fsResponsesLen = *fsResponsesLen;
    if (this->fsResponsesMaxLen < *fsResponsesLen) {
      this->fsResponsesMaxLen = this->fsResponsesLen;
    }
  }
  if (maxFsResponsesLen != nullptr) {
    this->fsResponsesMaxLen = *maxFsResponsesLen;
  }
  return returnvalue::OK;
}

ReturnValue_t FinishedInfo::getFilestoreResonses(FilestoreResponseTlv*** fsResponses,
                                                 size_t* fsResponsesLen,
                                                 size_t* fsResponsesMaxLen) {
  if (fsResponses == nullptr) {
    return returnvalue::FAILED;
  }
  *fsResponses = this->fsResponses;
  if (fsResponsesLen != nullptr) {
    *fsResponsesLen = this->fsResponsesLen;
  }
  if (fsResponsesMaxLen != nullptr) {
    *fsResponsesMaxLen = this->fsResponsesMaxLen;
  }
  return returnvalue::OK;
}

void FinishedInfo::setFaultLocation(EntityIdTlv* faultLocation) {
  this->faultLocation = faultLocation;
}

ReturnValue_t FinishedInfo::getFaultLocation(EntityIdTlv** faultLocation) {
  if (this->faultLocation == nullptr) {
    return returnvalue::FAILED;
  }
  *faultLocation = this->faultLocation;
  return returnvalue::OK;
}

cfdp::ConditionCode FinishedInfo::getConditionCode() const { return conditionCode; }

void FinishedInfo::setConditionCode(cfdp::ConditionCode conditionCode) {
  this->conditionCode = conditionCode;
}

cfdp::FileDeliveryCode FinishedInfo::getDeliveryCode() const { return deliveryCode; }

void FinishedInfo::setDeliveryCode(cfdp::FileDeliveryCode deliveryCode) {
  this->deliveryCode = deliveryCode;
}

cfdp::FileDeliveryStatus FinishedInfo::getFileStatus() const { return fileStatus; }

void FinishedInfo::setFilestoreResponsesArrayLen(size_t fsResponsesLen) {
  this->fsResponsesLen = fsResponsesLen;
}

size_t FinishedInfo::getFsResponsesLen() const { return fsResponsesLen; }

void FinishedInfo::setFileStatus(cfdp::FileDeliveryStatus fileStatus) {
  this->fileStatus = fileStatus;
}
