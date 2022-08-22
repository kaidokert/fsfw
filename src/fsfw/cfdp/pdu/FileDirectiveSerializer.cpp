#include "FileDirectiveSerializer.h"

FileDirectiveSerializer::FileDirectiveSerializer(PduConfig &pduConf,
                                                 cfdp::FileDirectives directiveCode,
                                                 size_t directiveParamFieldLen)
    : HeaderSerializer(pduConf, cfdp::PduType::FILE_DIRECTIVE, directiveParamFieldLen + 1),
      directiveCode(directiveCode) {}

size_t FileDirectiveSerializer::getSerializedSize() const {
  return HeaderSerializer::getSerializedSize() + 1;
}

ReturnValue_t FileDirectiveSerializer::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                                 Endianness streamEndianness) const {
  if (buffer == nullptr or size == nullptr) {
    return returnvalue::FAILED;
  }
  if (FileDirectiveSerializer::getWholePduSize() > maxSize) {
    return BUFFER_TOO_SHORT;
  }
  ReturnValue_t result = HeaderSerializer::serialize(buffer, size, maxSize, streamEndianness);
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

void FileDirectiveSerializer::setDirectiveDataFieldLen(size_t len) {
  // Set length of data field plus 1 byte for the directive octet
  HeaderSerializer::setPduDataFieldLen(len + 1);
}
