#include "PacketStorageHelper.h"

#include <cstring>

#include "fsfw/serialize.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

PacketStorageHelper::PacketStorageHelper(StorageManagerIF* store_,
                                         SerializeIF::Endianness endianness_)
    : store(store_), endianness(endianness_) {}

ReturnValue_t PacketStorageHelper::addPacket(SerializeIF* packet, store_address_t& storeId) {
  uint8_t* ptr = nullptr;
  size_t serLen = 0;
  ReturnValue_t result = store->getFreeElement(&storeId, packet->getSerializedSize(), &ptr);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return packet->serialize(&ptr, &serLen, packet->getSerializedSize(), endianness);
}

ReturnValue_t PacketStorageHelper::deletePacket(store_address_t storeId) {
  return store->deleteData(storeId);
}
