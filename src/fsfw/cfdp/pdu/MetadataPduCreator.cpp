#include "MetadataPduCreator.h"

MetadataPduCreator::MetadataPduCreator(PduConfig &conf, MetadataInfo &info)
    : FileDirectiveCreator(conf, cfdp::FileDirective::METADATA, 5), info(info) {
  updateDirectiveFieldLen();
}

void MetadataPduCreator::updateDirectiveFieldLen() {
  setDirectiveDataFieldLen(info.getSerializedSize(getLargeFileFlag()));
}

size_t MetadataPduCreator::getSerializedSize() const {
  return FileDirectiveCreator::getWholePduSize();
}

ReturnValue_t MetadataPduCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                            Endianness streamEndianness) const {
  ReturnValue_t result = FileDirectiveCreator::serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (*size + 1 >= maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  **buffer = info.isClosureRequested() << 6 | info.getChecksumType();
  *buffer += 1;
  *size += 1;
  result = info.getFileSize().serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = info.getSourceFileName().serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = info.getDestFileName().serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }

  if (info.hasOptions()) {
    cfdp::Tlv **optsArray = nullptr;
    size_t optsLen = 0;
    info.getOptions(&optsArray, &optsLen, nullptr);
    for (size_t idx = 0; idx < optsLen; idx++) {
      result = optsArray[idx]->serialize(buffer, size, maxSize, streamEndianness);
      if (result != returnvalue::OK) {
        return result;
      }
    }
  }
  return result;
}
