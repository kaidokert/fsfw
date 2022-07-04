#include "AckInfo.h"

AckInfo::AckInfo(cfdp::FileDirectives ackedDirective, cfdp::ConditionCode ackedConditionCode,
                 cfdp::AckTransactionStatus transactionStatus, uint8_t directiveSubtypeCode)
    : ackedDirective(ackedDirective),
      ackedConditionCode(ackedConditionCode),
      transactionStatus(transactionStatus),
      directiveSubtypeCode(directiveSubtypeCode) {
  if (ackedDirective == cfdp::FileDirectives::FINISH) {
    this->directiveSubtypeCode = 0b0001;
  } else {
    this->directiveSubtypeCode = 0b0000;
  }
}

AckInfo::AckInfo() = default;

cfdp::ConditionCode AckInfo::getAckedConditionCode() const { return ackedConditionCode; }

void AckInfo::setAckedConditionCode(cfdp::ConditionCode ackedConditionCode_) {
  ackedConditionCode = ackedConditionCode_;
  if (ackedDirective == cfdp::FileDirectives::FINISH) {
    this->directiveSubtypeCode = 0b0001;
  } else {
    this->directiveSubtypeCode = 0b0000;
  }
}

cfdp::FileDirectives AckInfo::getAckedDirective() const { return ackedDirective; }

void AckInfo::setAckedDirective(cfdp::FileDirectives ackedDirective_) {
  ackedDirective = ackedDirective_;
}

uint8_t AckInfo::getDirectiveSubtypeCode() const { return directiveSubtypeCode; }

void AckInfo::setDirectiveSubtypeCode(uint8_t directiveSubtypeCode_) {
  directiveSubtypeCode = directiveSubtypeCode_;
}

cfdp::AckTransactionStatus AckInfo::getTransactionStatus() const { return transactionStatus; }

void AckInfo::setTransactionStatus(cfdp::AckTransactionStatus transactionStatus_) {
  transactionStatus = transactionStatus_;
}
