#include "NakPduSerializer.h"

NakPduSerializer::NakPduSerializer(PduConfig &pduConf, NakInfo &nakInfo)
    : FileDirectiveSerializer(pduConf, cfdp::FileDirectives::NAK, 0), nakInfo(nakInfo) {
  updateDirectiveFieldLen();
}

void NakPduSerializer::updateDirectiveFieldLen() {
  this->setDirectiveDataFieldLen(nakInfo.getSerializedSize(getLargeFileFlag()));
}

size_t NakPduSerializer::getSerializedSize() const {
  return FileDirectiveSerializer::getWholePduSize();
}

ReturnValue_t NakPduSerializer::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                          Endianness streamEndianness) const {
  ReturnValue_t result =
      FileDirectiveSerializer::serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = nakInfo.getStartOfScope().serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = nakInfo.getEndOfScope().serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (nakInfo.hasSegmentRequests()) {
    NakInfo::SegmentRequest *segmentRequests = nullptr;
    size_t segmentRequestLen = 0;
    nakInfo.getSegmentRequests(&segmentRequests, &segmentRequestLen, nullptr);
    for (size_t idx = 0; idx < segmentRequestLen; idx++) {
      result = segmentRequests[idx].first.serialize(buffer, size, maxSize, streamEndianness);
      if (result != returnvalue::OK) {
        return result;
      }
      result = segmentRequests[idx].second.serialize(buffer, size, maxSize, streamEndianness);
      if (result != returnvalue::OK) {
        return result;
      }
    }
  }
  return result;
}
