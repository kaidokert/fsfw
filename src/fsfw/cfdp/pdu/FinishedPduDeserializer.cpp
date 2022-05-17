#include "FinishedPduDeserializer.h"

FinishPduDeserializer::FinishPduDeserializer(const uint8_t* pduBuf, size_t maxSize,
                                             FinishedInfo& info)
    : FileDirectiveDeserializer(pduBuf, maxSize), finishedInfo(info) {}

ReturnValue_t FinishPduDeserializer::parseData() {
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
  uint8_t firstByte = *buf;
  cfdp::ConditionCode condCode = static_cast<cfdp::ConditionCode>((firstByte >> 4) & 0x0f);
  finishedInfo.setConditionCode(condCode);
  finishedInfo.setDeliveryCode(static_cast<cfdp::FinishedDeliveryCode>(firstByte >> 2 & 0b1));
  finishedInfo.setFileStatus(static_cast<cfdp::FinishedFileStatus>(firstByte & 0b11));
  buf += 1;
  remSize -= 1;
  currentIdx += 1;
  if (remSize > 0) {
    result = parseTlvs(remSize, currentIdx, buf, condCode);
  }
  return result;
}

FinishedInfo& FinishPduDeserializer::getInfo() { return finishedInfo; }

ReturnValue_t FinishPduDeserializer::parseTlvs(size_t remLen, size_t currentIdx, const uint8_t* buf,
                                               cfdp::ConditionCode conditionCode) {
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  size_t fsResponsesIdx = 0;
  auto endianness = getEndianness();
  FilestoreResponseTlv** fsResponseArray = nullptr;
  size_t fsResponseMaxArrayLen = 0;
  EntityIdTlv* faultLocation = nullptr;
  cfdp::TlvTypes nextTlv = cfdp::TlvTypes::INVALID_TLV;
  while (remLen > 0) {
    // Simply forward parse the TLV type. Every TLV type except the last one must be a Filestore
    // Response TLV, and even the last one can be a Filestore Response TLV if the fault
    // location is omitted
    if (currentIdx + 2 > maxSize) {
      return SerializeIF::STREAM_TOO_SHORT;
    }
    nextTlv = static_cast<cfdp::TlvTypes>(*buf);
    if (nextTlv == cfdp::TlvTypes::FILESTORE_RESPONSE) {
      if (fsResponseArray == nullptr) {
        if (not finishedInfo.canHoldFsResponses()) {
          return cfdp::FINISHED_CANT_PARSE_FS_RESPONSES;
        }
        result =
            finishedInfo.getFilestoreResonses(&fsResponseArray, nullptr, &fsResponseMaxArrayLen);
      }
      if (fsResponsesIdx == fsResponseMaxArrayLen) {
        return cfdp::FINISHED_CANT_PARSE_FS_RESPONSES;
      }
      result = fsResponseArray[fsResponsesIdx]->deSerialize(&buf, &remLen, endianness);
      if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
      }
      fsResponsesIdx += 1;
    } else if (nextTlv == cfdp::TlvTypes::ENTITY_ID) {
      // This needs to be the last TLV and it should not be here if the condition code
      // is "No Error" or "Unsupported Checksum Type"
      if (conditionCode == cfdp::ConditionCode::NO_ERROR or
          conditionCode == cfdp::ConditionCode::UNSUPPORTED_CHECKSUM_TYPE) {
        return cfdp::INVALID_TLV_TYPE;
      }
      result = finishedInfo.getFaultLocation(&faultLocation);
      if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
      }
      result = faultLocation->deSerialize(&buf, &remLen, endianness);
      if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
      }
    } else {
      return cfdp::INVALID_TLV_TYPE;
    }
  }
  finishedInfo.setFilestoreResponsesArrayLen(fsResponsesIdx);
  return result;
}
