#include "KeepAlivePduReader.h"

KeepAlivePduReader::KeepAlivePduReader(const uint8_t* pduBuf, size_t maxSize,
                                       cfdp::FileSize& progress)
    : FileDirectiveReader(pduBuf, maxSize), progress(progress) {}

ReturnValue_t KeepAlivePduReader::parseData() {
  ReturnValue_t result = FileDirectiveReader::parseData();
  if (result != returnvalue::OK) {
    return result;
  }
  size_t currentIdx = FileDirectiveReader::getHeaderSize();
  size_t remLen = FileDirectiveReader::getWholePduSize() - currentIdx;
  const uint8_t* buffer = pointers.rawPtr + currentIdx;
  return progress.deSerialize(&buffer, &remLen, getEndianness());
}

cfdp::FileSize& KeepAlivePduReader::getProgress() { return progress; }
