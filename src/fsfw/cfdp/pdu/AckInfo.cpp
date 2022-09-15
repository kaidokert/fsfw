#include "AckInfo.h"

AckInfo::AckInfo(cfdp::FileDirectives ackedDirective, cfdp::ConditionCodes ackedConditionCode,
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

cfdp::ConditionCodes AckInfo::getAckedConditionCode() const { return ackedConditionCode; }

void AckInfo::setAckedConditionCode(cfdp::ConditionCodes ackedConditionCode) {
  this->ackedConditionCode = ackedConditionCode;
  if (ackedDirective == cfdp::FileDirectives::FINISH) {
    this->directiveSubtypeCode = 0b0001;
  } else {
    this->directiveSubtypeCode = 0b0000;
  }
}

cfdp::FileDirectives AckInfo::getAckedDirective() const { return ackedDirective; }

void AckInfo::setAckedDirective(cfdp::FileDirectives ackedDirective) {
  this->ackedDirective = ackedDirective;
}

uint8_t AckInfo::getDirectiveSubtypeCode() const { return directiveSubtypeCode; }

void AckInfo::setDirectiveSubtypeCode(uint8_t directiveSubtypeCode) {
  this->directiveSubtypeCode = directiveSubtypeCode;
}

cfdp::AckTransactionStatus AckInfo::getTransactionStatus() const { return transactionStatus; }

AckInfo::AckInfo() {}

void AckInfo::setTransactionStatus(cfdp::AckTransactionStatus transactionStatus) {
  this->transactionStatus = transactionStatus;
}
