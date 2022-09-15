#include "AckPduCreator.h"

AckPduCreator::AckPduCreator(AckInfo &ackInfo, PduConfig &pduConf)
    : FileDirectiveCreator(pduConf, cfdp::FileDirectives::ACK, 2), ackInfo(ackInfo) {}

size_t AckPduCreator::getSerializedSize() const { return FileDirectiveCreator::getWholePduSize(); }

ReturnValue_t AckPduCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                       Endianness streamEndianness) const {
  ReturnValue_t result = FileDirectiveCreator::serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  cfdp::FileDirectives ackedDirective = ackInfo.getAckedDirective();
  uint8_t directiveSubtypeCode = ackInfo.getDirectiveSubtypeCode();
  cfdp::ConditionCodes ackedConditionCode = ackInfo.getAckedConditionCode();
  cfdp::AckTransactionStatus transactionStatus = ackInfo.getTransactionStatus();
  if (ackedDirective != cfdp::FileDirectives::FINISH and
      ackedDirective != cfdp::FileDirectives::EOF_DIRECTIVE) {
    // TODO: better returncode
    return returnvalue::FAILED;
  }
  if (*size + 2 > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  **buffer = ackedDirective << 4 | directiveSubtypeCode;
  *buffer += 1;
  *size += 1;
  **buffer = ackedConditionCode << 4 | transactionStatus;
  *buffer += 1;
  *size += 1;
  return returnvalue::OK;
}
