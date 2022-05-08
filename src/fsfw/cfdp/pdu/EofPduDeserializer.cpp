#include "EofPduDeserializer.h"

#include "fsfw/FSFW.h"
#include "fsfw/serviceinterface.h"

EofPduDeserializer::EofPduDeserializer(const uint8_t* pduBuf, size_t maxSize, EofInfo& eofInfo)
    : FileDirectiveDeserializer(pduBuf, maxSize), info(eofInfo) {}

ReturnValue_t EofPduDeserializer::parseData() {
  ReturnValue_t result = FileDirectiveDeserializer::parseData();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }

  const uint8_t* bufPtr = rawPtr;
  size_t expectedFileFieldLen = 4;
  if (this->getLargeFileFlag()) {
    expectedFileFieldLen = 8;
  }
  size_t currentIdx = FileDirectiveDeserializer::getHeaderSize();
  size_t deserLen = maxSize;
  if (maxSize < currentIdx + 5 + expectedFileFieldLen) {
    return SerializeIF::STREAM_TOO_SHORT;
  }

  bufPtr += currentIdx;
  deserLen -= currentIdx;
  info.setConditionCode(static_cast<cfdp::ConditionCode>(*bufPtr >> 4));
  bufPtr += 1;
  deserLen -= 1;
  uint32_t checksum = 0;
  auto endianness = getEndianness();
  result = SerializeAdapter::deSerialize(&checksum, &bufPtr, &deserLen, endianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  info.setChecksum(checksum);
  if (this->getLargeFileFlag()) {
    uint64_t fileSizeValue = 0;
    result = SerializeAdapter::deSerialize(&fileSizeValue, &bufPtr, &deserLen, endianness);
    info.setFileSize(fileSizeValue, true);
  } else {
    uint32_t fileSizeValue = 0;
    result = SerializeAdapter::deSerialize(&fileSizeValue, &bufPtr, &deserLen, endianness);
    info.setFileSize(fileSizeValue, false);
  }
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  if (info.getConditionCode() != cfdp::ConditionCode::NO_ERROR) {
    EntityIdTlv* tlvPtr = info.getFaultLoc();
    if (tlvPtr == nullptr) {
      FSFW_LOGW("{}",
                "parseData: Ca not deserialize fault location,"
                " given TLV pointer invalid\n");
      return HasReturnvaluesIF::RETURN_FAILED;
    }
    result = tlvPtr->deSerialize(&bufPtr, &deserLen, endianness);
  }
  return result;
}
