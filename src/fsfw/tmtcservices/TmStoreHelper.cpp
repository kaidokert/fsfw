#include "TmStoreHelper.h"

#include "TmTcMessage.h"

TmStoreHelper::TmStoreHelper(uint16_t defaultApid, StorageManagerIF* tmStore,
                             TimeStamperIF* timeStamper)
    : creator(timeStamper), tmStore(tmStore) {
  creator.setApid(defaultApid);
}

TmStoreHelper::TmStoreHelper(uint16_t defaultApid, StorageManagerIF* tmStore)
    : creator(nullptr), tmStore(tmStore) {
  creator.setApid(defaultApid);
}

ReturnValue_t TmStoreHelper::preparePacket(uint8_t service, uint8_t subservice, uint16_t counter) {
  // TODO: Implement
  // creator.setApid(apid);
  PusTmParams& params = creator.getParams();
  params.secHeader.service = service;
  params.secHeader.subservice = subservice;
  params.secHeader.messageTypeCounter = counter;
  // TODO: Implement serialize and then serialize into the store
  return HasReturnvaluesIF::RETURN_OK;
}

StorageManagerIF* TmStoreHelper::getTmStore() { return tmStore; }

void TmStoreHelper::setTmStore(StorageManagerIF* store) { tmStore = store; }
const store_address_t& TmStoreHelper::getCurrentAddr() const { return currentAddr; }
ReturnValue_t TmStoreHelper::deletePacket() { return tmStore->deleteData(currentAddr); }

void TmStoreHelper::setSourceDataRaw(const uint8_t* data, size_t len) {
  PusTmParams& params = creator.getParams();
  params.dataWrapper.type = ecss::DataTypes::RAW;
  params.dataWrapper.dataUnion.raw.data = data;
  params.dataWrapper.dataUnion.raw.len = len;
}

void TmStoreHelper::setSourceDataSerializable(SerializeIF* serializable) {
  PusTmParams& params = creator.getParams();
  params.dataWrapper.type = ecss::DataTypes::SERIALIZABLE;
  params.dataWrapper.dataUnion.serializable = serializable;
}

ReturnValue_t TmStoreHelper::addPacketToStore() {
  creator.updateSpLengthField();
  uint8_t* dataPtr;
  tmStore->getFreeElement(&currentAddr, creator.getSerializedSize(), &dataPtr);
  size_t serLen = 0;
  return creator.serialize(&dataPtr, &serLen, creator.getSerializedSize(),
                           SerializeIF::Endianness::NETWORK);
}

void TmStoreHelper::setTimeStamper(TimeStamperIF* timeStamper_) {
  creator.setTimeStamper(timeStamper_);
}
void TmStoreHelper::setApid(uint16_t apid) { creator.setApid(apid); }
