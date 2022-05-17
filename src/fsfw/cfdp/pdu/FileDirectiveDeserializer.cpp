#include "FileDirectiveDeserializer.h"

FileDirectiveDeserializer::FileDirectiveDeserializer(const uint8_t *pduBuf, size_t maxSize)
    : HeaderDeserializer(pduBuf, maxSize) {}

cfdp::FileDirectives FileDirectiveDeserializer::getFileDirective() const { return fileDirective; }

ReturnValue_t FileDirectiveDeserializer::parseData() {
  ReturnValue_t result = HeaderDeserializer::parseData();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  if (this->getPduDataFieldLen() < 1) {
    return cfdp::INVALID_PDU_DATAFIELD_LEN;
  }
  if (FileDirectiveDeserializer::getWholePduSize() > maxSize) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  size_t currentIdx = HeaderDeserializer::getHeaderSize();
  if (not checkFileDirective(rawPtr[currentIdx])) {
    return cfdp::INVALID_DIRECTIVE_FIELDS;
  }
  setFileDirective(static_cast<cfdp::FileDirectives>(rawPtr[currentIdx]));
  return HasReturnvaluesIF::RETURN_OK;
}

size_t FileDirectiveDeserializer::getHeaderSize() const {
  // return size of header plus the directive byte
  return HeaderDeserializer::getHeaderSize() + 1;
}

bool FileDirectiveDeserializer::checkFileDirective(uint8_t rawByte) {
  if (rawByte < cfdp::FileDirectives::EOF_DIRECTIVE or
      (rawByte > cfdp::FileDirectives::PROMPT and rawByte != cfdp::FileDirectives::KEEP_ALIVE)) {
    // Invalid directive field. TODO: Custom returnvalue
    return false;
  }
  return true;
}

void FileDirectiveDeserializer::setFileDirective(cfdp::FileDirectives fileDirective) {
  this->fileDirective = fileDirective;
}

void FileDirectiveDeserializer::setEndianness(SerializeIF::Endianness endianness) {
  this->endianness = endianness;
}

SerializeIF::Endianness FileDirectiveDeserializer::getEndianness() const { return endianness; }
