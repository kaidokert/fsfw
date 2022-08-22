#include "KeepAlivePduDeserializer.h"

KeepAlivePduDeserializer::KeepAlivePduDeserializer(const uint8_t* pduBuf, size_t maxSize,
                                                   cfdp::FileSize& progress)
    : FileDirectiveDeserializer(pduBuf, maxSize), progress(progress) {}

ReturnValue_t KeepAlivePduDeserializer::parseData() {
  ReturnValue_t result = FileDirectiveDeserializer::parseData();
  if (result != returnvalue::OK) {
    return result;
  }
  size_t currentIdx = FileDirectiveDeserializer::getHeaderSize();
  size_t remLen = FileDirectiveDeserializer::getWholePduSize() - currentIdx;
  const uint8_t* buffer = rawPtr + currentIdx;
  return progress.deSerialize(&buffer, &remLen, getEndianness());
}

cfdp::FileSize& KeepAlivePduDeserializer::getProgress() { return progress; }
