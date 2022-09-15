#include "FileDirectiveReader.h"

FileDirectiveReader::FileDirectiveReader(const uint8_t *pduBuf, size_t maxSize)
    : PduHeaderReader(pduBuf, maxSize) {}

cfdp::FileDirective FileDirectiveReader::getFileDirective() const { return fileDirective; }

ReturnValue_t FileDirectiveReader::parseData() {
  ReturnValue_t result = PduHeaderReader::parseData();
  if (result != returnvalue::OK) {
    return result;
  }
  if (this->getPduDataFieldLen() < 1) {
    return cfdp::INVALID_PDU_DATAFIELD_LEN;
  }
  if (FileDirectiveReader::getWholePduSize() > maxSize) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  size_t currentIdx = PduHeaderReader::getHeaderSize();
  if (not checkFileDirective(pointers.rawPtr[currentIdx])) {
    return cfdp::INVALID_DIRECTIVE_FIELD;
  }
  setFileDirective(static_cast<cfdp::FileDirective>(pointers.rawPtr[currentIdx]));
  return returnvalue::OK;
}

size_t FileDirectiveReader::getHeaderSize() const {
  // return size of header plus the directive byte
  return PduHeaderReader::getHeaderSize() + 1;
}

bool FileDirectiveReader::checkFileDirective(uint8_t rawByte) {
  if (rawByte < cfdp::FileDirective::EOF_DIRECTIVE or
      (rawByte > cfdp::FileDirective::PROMPT and rawByte != cfdp::FileDirective::KEEP_ALIVE)) {
    // Invalid directive field
    return false;
  }
  return true;
}

void FileDirectiveReader::setFileDirective(cfdp::FileDirective fileDirective_) {
  fileDirective = fileDirective_;
}

void FileDirectiveReader::setEndianness(SerializeIF::Endianness endianness_) {
  endianness = endianness_;
}

SerializeIF::Endianness FileDirectiveReader::getEndianness() const { return endianness; }
