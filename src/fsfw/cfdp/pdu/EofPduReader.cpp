#include "EofPduReader.h"

#include "fsfw/FSFW.h"
#include "fsfw/serviceinterface.h"

EofPduReader::EofPduReader(const uint8_t* pduBuf, size_t maxSize, EofInfo& eofInfo)
    : FileDirectiveReader(pduBuf, maxSize), info(eofInfo) {}

ReturnValue_t EofPduReader::parseData() {
  ReturnValue_t result = FileDirectiveReader::parseData();
  if (result != returnvalue::OK) {
    return result;
  }

  const uint8_t* bufPtr = pointers.rawPtr;
  size_t expectedFileFieldLen = 4;
  if (this->getLargeFileFlag()) {
    expectedFileFieldLen = 8;
  }
  size_t currentIdx = FileDirectiveReader::getHeaderSize();
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
  if (result != returnvalue::OK) {
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
  if (result != returnvalue::OK) {
    return result;
  }
  if (info.getConditionCode() != cfdp::ConditionCode::NO_ERROR) {
    EntityIdTlv* tlvPtr = info.getFaultLoc();
    if (tlvPtr == nullptr) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "EofPduDeserializer::parseData: Ca not deserialize fault location,"
                      " given TLV pointer invalid"
                   << std::endl;
#else
      sif::printWarning(
          "EofPduDeserializer::parseData: Ca not deserialize fault location,"
          " given TLV pointer invalid");
#endif
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
      return returnvalue::FAILED;
    }
    result = tlvPtr->deSerialize(&bufPtr, &deserLen, endianness);
  }
  return result;
}
