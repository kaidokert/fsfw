#include "TmStoreHelper.h"

#include "TmTcMessage.h"

TmStoreHelper::TmStoreHelper(uint16_t defaultApid): tmStore(nullptr) {
  creator.setApid(defaultApid);
}

TmStoreHelper::TmStoreHelper(uint16_t defaultApid, StorageManagerIF& tmStore) : tmStore(&tmStore) {
  creator.setApid(defaultApid);
}

TmStoreHelper::TmStoreHelper(uint16_t defaultApid, StorageManagerIF& tmStore,
                             TimeStamperIF& timeStamper)
    : tmStore(&tmStore) {
  creator.setApid(defaultApid);
  creator.setTimeStamper(timeStamper);
}


ReturnValue_t TmStoreHelper::preparePacket(uint8_t service, uint8_t subservice, uint16_t counter) {
  PusTmParams& params = creator.getParams();
  params.secHeader.service = service;
  params.secHeader.subservice = subservice;
  params.secHeader.messageTypeCounter = counter;
  return HasReturnvaluesIF::RETURN_OK;
}

StorageManagerIF* TmStoreHelper::getTmStore() { return tmStore; }

void TmStoreHelper::setTmStore(StorageManagerIF& store) { tmStore = &store; }

const store_address_t& TmStoreHelper::getCurrentAddr() const { return currentAddr; }

ReturnValue_t TmStoreHelper::deletePacket() { return tmStore->deleteData(currentAddr); }

ReturnValue_t TmStoreHelper::setSourceDataRaw(const uint8_t* data, size_t len) {
  return creator.setRawUserData(data, len);
}

ReturnValue_t TmStoreHelper::setSourceDataSerializable(SerializeIF* serializable) {
  return creator.setSerializableUserData(serializable);
}

ReturnValue_t TmStoreHelper::addPacketToStore() {
  creator.updateSpLengthField();
  uint8_t* dataPtr;
  ReturnValue_t result =
      tmStore->getFreeElement(&currentAddr, creator.getSerializedSize(), &dataPtr);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  size_t serLen = 0;
  return creator.serialize(&dataPtr, &serLen, creator.getSerializedSize(),
                           SerializeIF::Endianness::NETWORK);
}

void TmStoreHelper::setTimeStamper(TimeStamperIF& timeStamper_) {
  creator.setTimeStamper(timeStamper_);
}

void TmStoreHelper::setApid(uint16_t apid) { creator.setApid(apid); }

PusTmCreator& TmStoreHelper::getCreatorRef() { return creator; }
