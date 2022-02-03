#include "PromptPduSerializer.h"

PromptPduSerializer::PromptPduSerializer(PduConfig &conf,
                                         cfdp::PromptResponseRequired responseRequired)
    : FileDirectiveSerializer(conf, cfdp::FileDirectives::PROMPT, 1),
      responseRequired(responseRequired) {}

size_t PromptPduSerializer::getSerializedSize() const {
  return FileDirectiveSerializer::getWholePduSize();
}

ReturnValue_t PromptPduSerializer::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                             Endianness streamEndianness) const {
  ReturnValue_t result =
      FileDirectiveSerializer::serialize(buffer, size, maxSize, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  if (*size + 1 > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  **buffer = this->responseRequired << 7;
  *buffer += 1;
  *size += 1;
  return result;
}
