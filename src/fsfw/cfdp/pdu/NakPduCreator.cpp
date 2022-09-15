#include "NakPduCreator.h"

NakPduCreator::NakPduCreator(PduConfig &pduConf, NakInfo &nakInfo)
    : FileDirectiveCreator(pduConf, cfdp::FileDirective::NAK, 0), nakInfo(nakInfo) {
  updateDirectiveFieldLen();
}

void NakPduCreator::updateDirectiveFieldLen() {
  this->setDirectiveDataFieldLen(nakInfo.getSerializedSize(getLargeFileFlag()));
}

size_t NakPduCreator::getSerializedSize() const { return FileDirectiveCreator::getWholePduSize(); }

ReturnValue_t NakPduCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                       Endianness streamEndianness) const {
  ReturnValue_t result = FileDirectiveCreator::serialize(buffer, size, maxSize, streamEndianness);
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
