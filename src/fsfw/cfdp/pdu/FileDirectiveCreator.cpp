#include "FileDirectiveCreator.h"

FileDirectiveCreator::FileDirectiveCreator(PduConfig &pduConf, cfdp::FileDirective directiveCode,
                                           size_t directiveParamFieldLen)
    : HeaderCreator(pduConf, cfdp::PduType::FILE_DIRECTIVE, directiveParamFieldLen + 1),
      directiveCode(directiveCode) {}

size_t FileDirectiveCreator::getSerializedSize() const {
  return HeaderCreator::getSerializedSize() + 1;
}

ReturnValue_t FileDirectiveCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                              Endianness streamEndianness) const {
  if (buffer == nullptr or size == nullptr) {
    return returnvalue::FAILED;
  }
  if (FileDirectiveCreator::getWholePduSize() > maxSize) {
    return BUFFER_TOO_SHORT;
  }
  ReturnValue_t result = HeaderCreator::serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }

  if (*size >= maxSize) {
    return BUFFER_TOO_SHORT;
  }
  **buffer = directiveCode;
  *buffer += 1;
  *size += 1;
  return returnvalue::OK;
}

void FileDirectiveCreator::setDirectiveDataFieldLen(size_t len) {
  // Set length of data field plus 1 byte for the directive octet
  HeaderCreator::setPduDataFieldLen(len + 1);
}

cfdp::FileDirective FileDirectiveCreator::getDirectiveCode() const { return directiveCode; }
