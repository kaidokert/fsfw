#include "PromptPduCreator.h"

PromptPduCreator::PromptPduCreator(PduConfig &conf, cfdp::PromptResponseRequired responseRequired)
    : FileDirectiveCreator(conf, cfdp::FileDirective::PROMPT, 1),
      responseRequired(responseRequired) {}

size_t PromptPduCreator::getSerializedSize() const {
  return FileDirectiveCreator::getWholePduSize();
}

ReturnValue_t PromptPduCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                          Endianness streamEndianness) const {
  ReturnValue_t result = FileDirectiveCreator::serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (*size + 1 > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  **buffer = responseRequired << 7;
  *buffer += 1;
  *size += 1;
  return result;
}
