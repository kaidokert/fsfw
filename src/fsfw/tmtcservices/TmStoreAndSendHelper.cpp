#include "TmStoreAndSendHelper.h"
#include "tmHelpers.h"

TmStoreAndSendWrapper::TmStoreAndSendWrapper(
    uint8_t defaultService, TmStoreHelper& storeHelper, TmSendHelper& sendHelper)
  : storeHelper(storeHelper), sendHelper(sendHelper), defaultService(defaultService){}

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
  if(incrementSendCounter) {
    sendCounter++;
  }
  return result;
}

ReturnValue_t TmStoreAndSendWrapper::sendTmPacket(uint8_t subservice, const uint8_t* sourceData,
                                                  size_t sourceDataLen) {
  storeHelper.preparePacket(defaultService, subservice, sendCounter);
  storeHelper.setSourceDataRaw(sourceData, sourceDataLen);
  return storeAndSendTmPacket();
}

ReturnValue_t TmStoreAndSendWrapper::sendTmPacket(uint8_t subservice, object_id_t objectId,
                                                  const uint8_t* data, size_t dataLen) {
  telemetry::DataWithObjectIdPrefix dataWithObjId(objectId, data, dataLen);
  storeHelper.preparePacket(defaultService, subservice, sendCounter);
  storeHelper.setSourceDataSerializable(dataWithObjId);
  return storeAndSendTmPacket();
}

ReturnValue_t TmStoreAndSendWrapper::sendTmPacket(uint8_t subservice, SerializeIF& sourceData) {
  storeHelper.preparePacket(defaultService, subservice, sendCounter);
  storeHelper.setSourceDataSerializable(sourceData);
  return storeAndSendTmPacket();
}

ReturnValue_t TmStoreAndSendWrapper::sendTmPacket(uint8_t subservice) {
  storeHelper.preparePacket(defaultService, subservice, sendCounter);
  storeHelper.setSourceDataRaw(nullptr, 0);
  return storeAndSendTmPacket();
}
