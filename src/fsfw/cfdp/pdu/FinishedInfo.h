#ifndef FSFW_SRC_FSFW_CFDP_PDU_FINISHINFO_H_
#define FSFW_SRC_FSFW_CFDP_PDU_FINISHINFO_H_

#include "../definitions.h"
#include "fsfw/cfdp/tlv/EntityIdTlv.h"
#include "fsfw/cfdp/tlv/FilestoreResponseTlv.h"

class FinishedInfo {
 public:
  FinishedInfo();
  FinishedInfo(cfdp::ConditionCode conditionCode, cfdp::FileDeliveryCode deliveryCode,
               cfdp::FileDeliveryStatus fileStatus);

  [[nodiscard]] size_t getSerializedSize() const;

  [[nodiscard]] bool hasFsResponses() const;
  [[nodiscard]] bool canHoldFsResponses() const;

  ReturnValue_t setFilestoreResponsesArray(FilestoreResponseTlv** fsResponses,
                                           size_t* fsResponsesLen, const size_t* maxFsResponseLen);
  void setFaultLocation(EntityIdTlv* entityId);

  ReturnValue_t getFilestoreResonses(FilestoreResponseTlv*** fsResponses, size_t* fsResponsesLen,
                                     size_t* fsResponsesMaxLen);
  [[nodiscard]] size_t getFsResponsesLen() const;
  void setFilestoreResponsesArrayLen(size_t fsResponsesLen);
  ReturnValue_t getFaultLocation(EntityIdTlv** entityId);
  [[nodiscard]] cfdp::ConditionCode getConditionCode() const;
  void setConditionCode(cfdp::ConditionCode conditionCode);
  [[nodiscard]] cfdp::FileDeliveryCode getDeliveryCode() const;
  void setDeliveryCode(cfdp::FileDeliveryCode deliveryCode);
  [[nodiscard]] cfdp::FileDeliveryStatus getFileStatus() const;
  void setFileStatus(cfdp::FileDeliveryStatus fileStatus);

 private:
  cfdp::ConditionCode conditionCode = cfdp::ConditionCode::NO_CONDITION_FIELD;
  cfdp::FileDeliveryCode deliveryCode = cfdp::FileDeliveryCode::DATA_COMPLETE;
  cfdp::FileDeliveryStatus fileStatus = cfdp::FileDeliveryStatus::DISCARDED_DELIBERATELY;
  FilestoreResponseTlv** fsResponses = nullptr;
  size_t fsResponsesLen = 0;
  size_t fsResponsesMaxLen = 0;
  EntityIdTlv* faultLocation = nullptr;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_FINISHINFO_H_ */
