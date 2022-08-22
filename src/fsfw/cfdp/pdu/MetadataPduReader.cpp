#include "MetadataPduReader.h"

MetadataPduReader::MetadataPduReader(const uint8_t* pduBuf, size_t maxSize, MetadataInfo& info)
    : FileDirectiveReader(pduBuf, maxSize), info(info) {}

ReturnValue_t MetadataPduReader::parseData() {
  ReturnValue_t result = FileDirectiveReader::parseData();
  if (result != returnvalue::OK) {
    return result;
  }
  size_t currentIdx = FileDirectiveReader::getHeaderSize();
  const uint8_t* buf = pointers.rawPtr + currentIdx;
  size_t remSize = FileDirectiveReader::getWholePduSize() - currentIdx;
  if (remSize < 1) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  info.setClosureRequested((*buf >> 6) & 0x01);
  info.setChecksumType(static_cast<cfdp::ChecksumType>(*buf & 0x0f));
  remSize -= 1;
  buf += 1;
  auto endianness = getEndianness();
  result = info.getFileSize().deSerialize(&buf, &remSize, endianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = info.getSourceFileName().deSerialize(&buf, &remSize, endianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = info.getDestFileName().deSerialize(&buf, &remSize, endianness);
  if (result != returnvalue::OK) {
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
      if (result != returnvalue::OK) {
        return result;
      }
      optsIdx++;
    }
    info.setOptionsLen(optsIdx);
  }
  return result;
}
