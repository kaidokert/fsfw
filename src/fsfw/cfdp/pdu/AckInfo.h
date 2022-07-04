#ifndef FSFW_SRC_FSFW_CFDP_PDU_ACKINFO_H_
#define FSFW_SRC_FSFW_CFDP_PDU_ACKINFO_H_

#include "../definitions.h"

class AckInfo {
 public:
  AckInfo();
  AckInfo(cfdp::FileDirectives ackedDirective, cfdp::ConditionCode ackedConditionCode,
          cfdp::AckTransactionStatus transactionStatus, uint8_t directiveSubtypeCode = 0);

  [[nodiscard]] cfdp::ConditionCode getAckedConditionCode() const;
  void setAckedConditionCode(cfdp::ConditionCode ackedConditionCode_);

  [[nodiscard]] cfdp::FileDirectives getAckedDirective() const;
  void setAckedDirective(cfdp::FileDirectives ackedDirective);

  [[nodiscard]] uint8_t getDirectiveSubtypeCode() const;
  void setDirectiveSubtypeCode(uint8_t directiveSubtypeCode);

  [[nodiscard]] cfdp::AckTransactionStatus getTransactionStatus() const;
  void setTransactionStatus(cfdp::AckTransactionStatus transactionStatus);

 private:
  cfdp::FileDirectives ackedDirective = cfdp::FileDirectives::INVALID_DIRECTIVE;
  cfdp::ConditionCode ackedConditionCode = cfdp::ConditionCode::NO_CONDITION_FIELD;
  cfdp::AckTransactionStatus transactionStatus = cfdp::AckTransactionStatus::UNDEFINED;
  uint8_t directiveSubtypeCode = 0;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_ACKINFO_H_ */
