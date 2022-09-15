#ifndef FSFW_SRC_FSFW_CFDP_PDU_ACKINFO_H_
#define FSFW_SRC_FSFW_CFDP_PDU_ACKINFO_H_

#include "../definitions.h"

class AckInfo {
 public:
  AckInfo();
  AckInfo(cfdp::FileDirective ackedDirective, cfdp::ConditionCode ackedConditionCode,
          cfdp::AckTransactionStatus transactionStatus, uint8_t directiveSubtypeCode = 0);

  cfdp::ConditionCode getAckedConditionCode() const;
  void setAckedConditionCode(cfdp::ConditionCode ackedConditionCode);

  cfdp::FileDirective getAckedDirective() const;
  void setAckedDirective(cfdp::FileDirective ackedDirective);

  uint8_t getDirectiveSubtypeCode() const;
  void setDirectiveSubtypeCode(uint8_t directiveSubtypeCode);

  cfdp::AckTransactionStatus getTransactionStatus() const;
  void setTransactionStatus(cfdp::AckTransactionStatus transactionStatus);

 private:
  cfdp::FileDirective ackedDirective = cfdp::FileDirective::INVALID_DIRECTIVE;
  cfdp::ConditionCode ackedConditionCode = cfdp::ConditionCode::NO_CONDITION_FIELD;
  cfdp::AckTransactionStatus transactionStatus = cfdp::AckTransactionStatus::UNDEFINED;
  uint8_t directiveSubtypeCode = 0;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_ACKINFO_H_ */
