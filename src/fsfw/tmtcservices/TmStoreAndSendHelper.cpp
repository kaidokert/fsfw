#include "TmStoreAndSendHelper.h"

#include "tmHelpers.h"

TmStoreAndSendWrapper::TmStoreAndSendWrapper(uint8_t defaultService, TmStoreHelper& storeHelper,
                                             TmSendHelper& sendHelper)
    : storeHelper(storeHelper), sendHelper(sendHelper), defaultService(defaultService) {}

ReturnValue_t TmStoreAndSendWrapper::storeAndSendTmPacket() {
  ReturnValue_t result = storeHelper.addPacketToStore();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = sendHelper.sendPacket(storeHelper.getCurrentAddr());
  if (result != HasReturnvaluesIF::RETURN_OK) {
    if (delOnFailure) {
      storeHelper.deletePacket();
    }
    return result;
  }
  if (incrementSendCounter) {
    sendCounter++;
  }
  return result;
}

ReturnValue_t TmStoreAndSendWrapper::prepareTmPacket(uint8_t subservice, const uint8_t* sourceData,
                                                     size_t sourceDataLen) {
  ReturnValue_t result = storeHelper.preparePacket(defaultService, subservice, sendCounter);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return storeHelper.setSourceDataRaw(sourceData, sourceDataLen);
}

ReturnValue_t TmStoreAndSendWrapper::prepareTmPacket(
    uint8_t subservice, telemetry::DataWithObjectIdPrefix& dataWithObjectId) {
  ReturnValue_t result = storeHelper.preparePacket(defaultService, subservice, sendCounter);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return storeHelper.setSourceDataSerializable(dataWithObjectId);
}

ReturnValue_t TmStoreAndSendWrapper::prepareTmPacket(uint8_t subservice, SerializeIF& sourceData) {
  ReturnValue_t result = storeHelper.preparePacket(defaultService, subservice, sendCounter);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return storeHelper.setSourceDataSerializable(sourceData);
}

ReturnValue_t TmStoreAndSendWrapper::prepareTmPacket(uint8_t subservice) {
  ReturnValue_t result = storeHelper.preparePacket(defaultService, subservice, sendCounter);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return storeHelper.setSourceDataRaw(nullptr, 0);
}
