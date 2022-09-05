#include "TmStoreHelper.h"

#include "TmTcMessage.h"

TmStoreHelper::TmStoreHelper(uint16_t defaultApid) : tmStore(nullptr) {
  creator.setApid(defaultApid);
}

TmStoreHelper::TmStoreHelper(uint16_t defaultApid, StorageManagerIF& tmStore) : tmStore(&tmStore) {
  creator.setApid(defaultApid);
}

TmStoreHelper::TmStoreHelper(uint16_t defaultApid, StorageManagerIF& tmStore,
                             TimeWriterIF& timeStamper)
    : tmStore(&tmStore) {
  creator.setApid(defaultApid);
  creator.setTimeStamper(timeStamper);
}

ReturnValue_t TmStoreHelper::preparePacket(uint8_t service, uint8_t subservice, uint16_t counter) {
  creator.setService(service);
  creator.setSubservice(subservice);
  creator.setMessageTypeCounter(counter);
  return returnvalue::OK;
}

StorageManagerIF* TmStoreHelper::getTmStore() const { return tmStore; }

void TmStoreHelper::setTmStore(StorageManagerIF& store) { tmStore = &store; }

const store_address_t& TmStoreHelper::getCurrentAddr() const { return currentAddr; }

ReturnValue_t TmStoreHelper::deletePacket() {
  ReturnValue_t result = tmStore->deleteData(currentAddr);
  if (result == returnvalue::OK) {
    currentAddr = store_address_t::invalid();
  }
  return result;
}

ReturnValue_t TmStoreHelper::setSourceDataRaw(const uint8_t* data, size_t len) {
  return creator.setRawUserData(data, len);
}

ReturnValue_t TmStoreHelper::setSourceDataSerializable(SerializeIF& serializable) {
  return creator.setSerializableUserData(serializable);
}

ReturnValue_t TmStoreHelper::addPacketToStore() {
  creator.updateSpLengthField();
  uint8_t* dataPtr;
  ReturnValue_t result =
      tmStore->getFreeElement(&currentAddr, creator.getSerializedSize(), &dataPtr);
  if (result != returnvalue::OK) {
    return result;
  }
  size_t serLen = 0;
  return creator.serialize(&dataPtr, &serLen, creator.getSerializedSize(),
                           SerializeIF::Endianness::NETWORK);
}

void TmStoreHelper::setTimeStamper(TimeWriterIF& timeStamper_) {
  creator.setTimeStamper(timeStamper_);
}

void TmStoreHelper::setApid(uint16_t apid) { creator.setApid(apid); }

PusTmCreator& TmStoreHelper::getCreatorRef() { return creator; }

TimeWriterIF* TmStoreHelper::getTimeStamper() const { return creator.getTimestamper(); }

uint16_t TmStoreHelper::getApid() const { return creator.getApid(); }

void TmStoreHelper::setService(uint8_t service) { creator.setService(service); }

void TmStoreHelper::setSubservice(uint8_t subservice) { creator.setSubservice(subservice); }

void TmStoreHelper::disableCrcCalculation() { creator.disableCrcCalculation(); }

bool TmStoreHelper::crcCalculationEnabled() const { return creator.crcCalculationEnabled(); }
