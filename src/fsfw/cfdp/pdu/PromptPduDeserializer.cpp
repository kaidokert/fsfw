#include "PromptPduDeserializer.h"

PromptPduDeserializer::PromptPduDeserializer(const uint8_t *pduBuf, size_t maxSize)
    : FileDirectiveReader(pduBuf, maxSize) {}

cfdp::PromptResponseRequired PromptPduDeserializer::getPromptResponseRequired() const {
  return responseRequired;
}

ReturnValue_t PromptPduDeserializer::parseData() {
  ReturnValue_t result = FileDirectiveReader::parseData();
  if (result != returnvalue::OK) {
    return result;
  }
  if (FileDirectiveReader::getWholePduSize() <= FileDirectiveReader::getHeaderSize()) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  responseRequired = static_cast<cfdp::PromptResponseRequired>(
      (pointers.rawPtr[FileDirectiveReader::getHeaderSize()] >> 7) & 0x01);
  return returnvalue::OK;
}
