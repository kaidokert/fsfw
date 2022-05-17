#include "NakPduDeserializer.h"

NakPduDeserializer::NakPduDeserializer(const uint8_t* pduBuf, size_t maxSize, NakInfo& info)
    : FileDirectiveDeserializer(pduBuf, maxSize), nakInfo(info) {}

ReturnValue_t NakPduDeserializer::parseData() {
  ReturnValue_t result = FileDirectiveDeserializer::parseData();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  size_t currentIdx = FileDirectiveDeserializer::getHeaderSize();
  const uint8_t* buffer = rawPtr + currentIdx;
  size_t remSize = FileDirectiveDeserializer::getWholePduSize() - currentIdx;
  if (remSize < 1) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  result =
      nakInfo.getStartOfScope().deSerialize(&buffer, &remSize, SerializeIF::Endianness::NETWORK);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = nakInfo.getEndOfScope().deSerialize(&buffer, &remSize, SerializeIF::Endianness::NETWORK);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  nakInfo.setSegmentRequestLen(0);
  if (remSize > 0) {
    if (not nakInfo.canHoldSegmentRequests()) {
      return cfdp::NAK_CANT_PARSE_OPTIONS;
    }
    NakInfo::SegmentRequest* segReqs = nullptr;
    size_t maxSegReqs = 0;
    nakInfo.getSegmentRequests(&segReqs, nullptr, &maxSegReqs);
    if (segReqs != nullptr) {
      size_t idx = 0;
      while (remSize > 0) {
        if (idx == maxSegReqs) {
          return cfdp::NAK_CANT_PARSE_OPTIONS;
        }
        result =
            segReqs[idx].first.deSerialize(&buffer, &remSize, SerializeIF::Endianness::NETWORK);
        if (result != HasReturnvaluesIF::RETURN_OK) {
          return result;
        }
        result =
            segReqs[idx].second.deSerialize(&buffer, &remSize, SerializeIF::Endianness::NETWORK);
        if (result != HasReturnvaluesIF::RETURN_OK) {
          return result;
        }
        idx++;
      }
      nakInfo.setSegmentRequestLen(idx);
    }
  }
  return result;
}
