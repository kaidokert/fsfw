#include "MetadataPduSerializer.h"

MetadataPduSerializer::MetadataPduSerializer(PduConfig &conf, MetadataInfo &info)
    : FileDirectiveSerializer(conf, cfdp::FileDirectives::METADATA, 5), info(info) {
  updateDirectiveFieldLen();
}

void MetadataPduSerializer::updateDirectiveFieldLen() {
  setDirectiveDataFieldLen(info.getSerializedSize(getLargeFileFlag()));
}

size_t MetadataPduSerializer::getSerializedSize() const {
  return FileDirectiveSerializer::getWholePduSize();
}

ReturnValue_t MetadataPduSerializer::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                               Endianness streamEndianness) const {
  ReturnValue_t result =
      FileDirectiveSerializer::serialize(buffer, size, maxSize, streamEndianness);
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
