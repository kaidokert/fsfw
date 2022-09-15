#include "EofPduCreator.h"

#include "fsfw/FSFW.h"

EofPduCreator::EofPduCreator(PduConfig &conf, EofInfo &info)
    : FileDirectiveCreator(conf, cfdp::FileDirective::EOF_DIRECTIVE, 9), info(info) {
  setDirectiveDataFieldLen(info.getSerializedSize(HeaderCreator::getLargeFileFlag()));
}

size_t EofPduCreator::getSerializedSize() const { return FileDirectiveCreator::getWholePduSize(); }

ReturnValue_t EofPduCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                       Endianness streamEndianness) const {
  ReturnValue_t result = FileDirectiveCreator::serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (*size + 1 > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  **buffer = info.getConditionCode() << 4;
  *buffer += 1;
  *size += 1;
  uint32_t checksum = info.getChecksum();
  result = SerializeAdapter::serialize(&checksum, buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (info.getFileSize().isLargeFile()) {
    uint64_t fileSizeValue = info.getFileSize().getSize();
    result = SerializeAdapter::serialize(&fileSizeValue, buffer, size, maxSize, streamEndianness);
  } else {
    uint32_t fileSizeValue = info.getFileSize().getSize();
    result = SerializeAdapter::serialize(&fileSizeValue, buffer, size, maxSize, streamEndianness);
  }
  if (info.getFaultLoc() != nullptr and info.getConditionCode() != cfdp::ConditionCode::NO_ERROR) {
    result = info.getFaultLoc()->serialize(buffer, size, maxSize, streamEndianness);
  }
  return result;
}
