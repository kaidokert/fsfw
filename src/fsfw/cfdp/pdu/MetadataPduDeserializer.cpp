#include "MetadataPduDeserializer.h"

MetadataPduDeserializer::MetadataPduDeserializer(const uint8_t* pduBuf, size_t maxSize,
                                                 MetadataInfo& info)
    : FileDirectiveDeserializer(pduBuf, maxSize), info(info) {}

ReturnValue_t MetadataPduDeserializer::parseData() {
  ReturnValue_t result = FileDirectiveDeserializer::parseData();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  size_t currentIdx = FileDirectiveDeserializer::getHeaderSize();
  const uint8_t* buf = rawPtr + currentIdx;
  size_t remSize = FileDirectiveDeserializer::getWholePduSize() - currentIdx;
  if (remSize < 1) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  info.setClosureRequested((*buf >> 6) & 0x01);
  info.setChecksumType(static_cast<cfdp::ChecksumType>(*buf & 0x0f));
  remSize -= 1;
  buf += 1;
  auto endianness = getEndianness();
  result = info.getFileSize().deSerialize(&buf, &remSize, endianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = info.getSourceFileName().deSerialize(&buf, &remSize, endianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = info.getDestFileName().deSerialize(&buf, &remSize, endianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }

  info.setOptionsLen(0);
  if (remSize > 0) {
    if (not info.canHoldOptions()) {
      return cfdp::METADATA_CANT_PARSE_OPTIONS;
    }
    cfdp::Tlv** optionsArray = nullptr;
    size_t optsMaxLen = 0;
    size_t optsIdx = 0;
    info.getOptions(&optionsArray, nullptr, &optsMaxLen);
    while (remSize > 0) {
      if (optsIdx > optsMaxLen) {
        return cfdp::METADATA_CANT_PARSE_OPTIONS;
      }
      result = optionsArray[optsIdx]->deSerialize(&buf, &remSize, endianness);
      if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
      }
      optsIdx++;
    }
    info.setOptionsLen(optsIdx);
  }
  return result;
}
