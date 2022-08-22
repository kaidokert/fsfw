#include "FileDirectiveReader.h"

FileDirectiveReader::FileDirectiveReader(const uint8_t *pduBuf, size_t maxSize)
    : HeaderReader(pduBuf, maxSize) {}

cfdp::FileDirectives FileDirectiveReader::getFileDirective() const { return fileDirective; }

ReturnValue_t FileDirectiveReader::parseData() {
  ReturnValue_t result = HeaderReader::parseData();
  if (result != returnvalue::OK) {
    return result;
  }
  if (this->getPduDataFieldLen() < 1) {
    return cfdp::INVALID_PDU_DATAFIELD_LEN;
  }
  if (FileDirectiveReader::getWholePduSize() > maxSize) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  size_t currentIdx = HeaderReader::getHeaderSize();
  if (not checkFileDirective(pointers.rawPtr[currentIdx])) {
    return cfdp::INVALID_DIRECTIVE_FIELDS;
  }
  setFileDirective(static_cast<cfdp::FileDirectives>(pointers.rawPtr[currentIdx]));
  return returnvalue::OK;
}

size_t FileDirectiveReader::getHeaderSize() const {
  // return size of header plus the directive byte
  return HeaderReader::getHeaderSize() + 1;
}

bool FileDirectiveReader::checkFileDirective(uint8_t rawByte) {
  if (rawByte < cfdp::FileDirectives::EOF_DIRECTIVE or
      (rawByte > cfdp::FileDirectives::PROMPT and rawByte != cfdp::FileDirectives::KEEP_ALIVE)) {
    // Invalid directive field. TODO: Custom returnvalue
    return false;
  }
  return true;
}

void FileDirectiveReader::setFileDirective(cfdp::FileDirectives fileDirective_) {
  fileDirective = fileDirective_;
}

void FileDirectiveReader::setEndianness(SerializeIF::Endianness endianness_) {
  endianness = endianness_;
}

SerializeIF::Endianness FileDirectiveReader::getEndianness() const { return endianness; }
