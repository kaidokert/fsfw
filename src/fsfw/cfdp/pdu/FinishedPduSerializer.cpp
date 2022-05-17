#include "FinishedPduSerializer.h"

FinishPduSerializer::FinishPduSerializer(PduConfig &conf, FinishedInfo &finishInfo)
    : FileDirectiveSerializer(conf, cfdp::FileDirectives::FINISH, 0), finishInfo(finishInfo) {
  updateDirectiveFieldLen();
}

size_t FinishPduSerializer::getSerializedSize() const {
  return FinishPduSerializer::getWholePduSize();
}

void FinishPduSerializer::updateDirectiveFieldLen() {
  setDirectiveDataFieldLen(finishInfo.getSerializedSize());
}

ReturnValue_t FinishPduSerializer::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                             Endianness streamEndianness) const {
  ReturnValue_t result =
      FileDirectiveSerializer::serialize(buffer, size, maxSize, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  if (*size + 1 >= maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  **buffer = finishInfo.getConditionCode() << 4 | finishInfo.getDeliveryCode() << 2 |
             finishInfo.getFileStatus();
  *size += 1;
  *buffer += 1;

  if (finishInfo.hasFsResponses()) {
    FilestoreResponseTlv **fsResponsesArray = nullptr;
    size_t fsResponsesArrayLen = 0;
    finishInfo.getFilestoreResonses(&fsResponsesArray, &fsResponsesArrayLen, nullptr);
    for (size_t idx = 0; idx < fsResponsesArrayLen; idx++) {
      result = fsResponsesArray[idx]->serialize(buffer, size, maxSize, streamEndianness);
      if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
      }
    }
  }
  EntityIdTlv *entityId = nullptr;
  if (finishInfo.getFaultLocation(&entityId) == HasReturnvaluesIF::RETURN_OK) {
    result = entityId->serialize(buffer, size, maxSize, streamEndianness);
  }
  return result;
}
