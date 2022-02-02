#include "AckPduDeserializer.h"

AckPduDeserializer::AckPduDeserializer(const uint8_t* pduBuf, size_t maxSize, AckInfo& info)
    : FileDirectiveDeserializer(pduBuf, maxSize), info(info) {}

ReturnValue_t AckPduDeserializer::parseData() {
  ReturnValue_t result = FileDirectiveDeserializer::parseData();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  size_t currentIdx = FileDirectiveDeserializer::getHeaderSize();
  if (currentIdx + 2 > this->maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  if (not checkAndSetCodes(rawPtr[currentIdx], rawPtr[currentIdx + 1])) {
    return cfdp::INVALID_ACK_DIRECTIVE_FIELDS;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

bool AckPduDeserializer::checkAndSetCodes(uint8_t firstByte, uint8_t secondByte) {
  uint8_t ackedDirective = static_cast<cfdp::FileDirectives>(firstByte >> 4);

  if (ackedDirective != cfdp::FileDirectives::EOF_DIRECTIVE and
      ackedDirective != cfdp::FileDirectives::FINISH) {
    return false;
  }
  this->info.setAckedDirective(static_cast<cfdp::FileDirectives>(ackedDirective));
  uint8_t directiveSubtypeCode = firstByte & 0x0f;
  if (directiveSubtypeCode != 0b0000 and directiveSubtypeCode != 0b0001) {
    return false;
  }
  this->info.setDirectiveSubtypeCode(directiveSubtypeCode);
  this->info.setAckedConditionCode(static_cast<cfdp::ConditionCode>(secondByte >> 4));
  this->info.setTransactionStatus(static_cast<cfdp::AckTransactionStatus>(secondByte & 0x0f));
  return true;
}
