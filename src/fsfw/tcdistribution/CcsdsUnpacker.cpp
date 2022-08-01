#include "CcsdsUnpacker.h"

#include "fsfw/tmtcservices/TmTcMessage.h"

CcsdsUnpacker::CcsdsUnpacker(MessageQueueIF& msgQueue, AcceptsTelecommandsIF& receiver,
                             StorageManagerIF& sourceStore)
  : sourceStore(sourceStore), msgQueue(msgQueue), receiver(receiver) {
  msgQueue.setDefaultDestination(receiver.getRequestQueue());
}

ReturnValue_t CcsdsUnpacker::performOperation(uint8_t operationCode) {
  TmTcMessage msg;
  ReturnValue_t result;
  for (result =  msgQueue.receiveMessage(&msg); result == HasReturnvaluesIF::RETURN_OK;
       result = msgQueue.receiveMessage(&msg)) {
    auto resultPair = sourceStore.getData(msg.getStorageId());
    if(resultPair.first != HasReturnvaluesIF::RETURN_OK) {
      continue;
    }
    if(resultPair.second.size() < 6) {
      // TODO: This is a config error. Does it make sense to forward the message?
      result = msgQueue.sendToDefault(&msg);
      if(result != HasReturnvaluesIF::RETURN_OK) {

      }
      continue;
    }
    StorageManagerIF* tgtStore;
    if(targetStore != nullptr) {
      tgtStore = targetStore;
    } else {
      tgtStore = &sourceStore;
    }
    store_address_t newId;
    uint8_t* ptr;
    result = tgtStore->getFreeElement(&newId, resultPair.second.size(), &ptr);
    if(result != HasReturnvaluesIF::RETURN_OK) {
      // TODO: Implement error handling
    }
    std::memcpy(ptr, resultPair.second.data() + 6, resultPair.second.size() - 6);
    result = sourceStore.deleteData(msg.getStorageId());
    if(result != HasReturnvaluesIF::RETURN_OK) {
      // TODO: Implement error handling (though this really should not happen)
    }
    TmTcMessage newMsg(newId);
    result = msgQueue.sendToDefault(&newMsg);
    if(result != HasReturnvaluesIF::RETURN_OK) {

    }
  }
  return result;
}


void CcsdsUnpacker::setDifferentTargetStore(StorageManagerIF& otherTargetStore) {
  targetStore = &otherTargetStore;
}
ReturnValue_t CcsdsUnpacker::performOperation(uint8_t operationCode) { return 0; }
uint32_t CcsdsUnpacker::getIdentifier() const { return 0; }
MessageQueueId_t CcsdsUnpacker::getRequestQueue() const { return 0; }
