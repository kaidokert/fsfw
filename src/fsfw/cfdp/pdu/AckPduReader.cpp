#include "AckPduReader.h"

AckPduReader::AckPduReader(const uint8_t* pduBuf, size_t maxSize, AckInfo& info)
    : FileDirectiveReader(pduBuf, maxSize), info(info) {}

ReturnValue_t AckPduReader::parseData() {
  ReturnValue_t result = FileDirectiveReader::parseData();
  if (result != returnvalue::OK) {
    return result;
  }
  size_t currentIdx = FileDirectiveReader::getHeaderSize();
  if (currentIdx + 2 > this->maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  if (not checkAndSetCodes(pointers.rawPtr[currentIdx], pointers.rawPtr[currentIdx + 1])) {
    return cfdp::INVALID_ACK_DIRECTIVE_FIELDS;
  }
  return returnvalue::OK;
}

bool AckPduReader::checkAndSetCodes(uint8_t firstByte, uint8_t secondByte) {
  cfdp::FileDirectives directive;
  if (not checkAckedDirectiveField(firstByte, directive)) {
    return false;
  }
  this->info.setAckedDirective(directive);
  uint8_t directiveSubtypeCode = firstByte & 0x0f;
  if (directiveSubtypeCode != 0b0000 and directiveSubtypeCode != 0b0001) {
    return false;
  }
  this->info.setDirectiveSubtypeCode(directiveSubtypeCode);
  this->info.setAckedConditionCode(static_cast<cfdp::ConditionCodes>(secondByte >> 4));
  this->info.setTransactionStatus(static_cast<cfdp::AckTransactionStatus>(secondByte & 0x0f));
  return true;
}
bool AckPduReader::checkAckedDirectiveField(uint8_t firstPduDataByte,
                                            cfdp::FileDirectives& ackedDirective) {
  uint8_t ackedDirectiveRaw = static_cast<cfdp::FileDirectives>(firstPduDataByte >> 4);
  if (ackedDirectiveRaw != cfdp::FileDirectives::EOF_DIRECTIVE and
      ackedDirectiveRaw != cfdp::FileDirectives::FINISH) {
    return false;
  }
  ackedDirective = (static_cast<cfdp::FileDirectives>(ackedDirectiveRaw));
  return true;
}
