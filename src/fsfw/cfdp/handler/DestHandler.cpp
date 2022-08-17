#include "DestHandler.h"

#include <utility>

#include "fsfw/objectmanager.h"

cfdp::DestHandler::DestHandler(DestHandlerParams params) : p(std::move(params)) {}

ReturnValue_t cfdp::DestHandler::performStateMachine() {
  switch (step) {
    case TransactionStep::IDLE: {
      for (const auto& info : p.packetListRef) {
      }
    }
    case TransactionStep::TRANSACTION_START:
      break;
    case TransactionStep::RECEIVING_FILE_DATA_PDUS:
      break;
    case TransactionStep::SENDING_ACK_PDU:
      break;
    case TransactionStep::TRANSFER_COMPLETION:
      break;
    case TransactionStep::SENDING_FINISHED_PDU:
      break;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t cfdp::DestHandler::passPacket(PacketInfo packet) {
  if (p.packetListRef.full()) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  p.packetListRef.push_back(packet);
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t cfdp::DestHandler::initialize() {
  if (p.tmStore == nullptr) {
    p.tmStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TM_STORE);
    if (p.tmStore == nullptr) {
      return HasReturnvaluesIF::RETURN_FAILED;
    }
  }

  if (p.tcStore == nullptr) {
    p.tcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
    if (p.tcStore == nullptr) {
      return HasReturnvaluesIF::RETURN_FAILED;
    }
  }
  return HasReturnvaluesIF::RETURN_OK;
}
