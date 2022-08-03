#include "CfdpRouter.h"

#include <algorithm>

#include "fsfw/tcdistribution/definitions.h"

CfdpRouter::CfdpRouter(CfdpRouterCfg cfg) : TcDistributorBase(cfg.objectId), tmQueue(cfg.tmQueue) {}

ReturnValue_t CfdpRouter::registerTmSink(cfdp::EntityId address, AcceptsTelemetryIF& tmDest) {
  for (const auto& dest : tmDestinations) {
    if (dest.id == address) {
      return HasReturnvaluesIF::RETURN_FAILED;
    }
  }
  tmDestinations.emplace_back(address, tmDest.getName(), tmDest.getReportReceptionQueue());
  return HasReturnvaluesIF::RETURN_OK;
}
ReturnValue_t CfdpRouter::registerTcDestination(cfdp::EntityId address,
                                                AcceptsTelecommandsIF& tcDest) {
  for (const auto& dest : tcDestinations) {
    if (dest.id == address) {
      return HasReturnvaluesIF::RETURN_FAILED;
    }
  }
  tcDestinations.emplace_back(address, tcDest.getName(), tcDest.getRequestQueue());
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CfdpRouter::selectDestination(MessageQueueId_t& destId) { return 0; }

const char* CfdpRouter::getName() const { return "CFDP Router"; }

uint32_t CfdpRouter::getIdentifier() const { return 0; }

MessageQueueId_t CfdpRouter::getRequestQueue() const { return tcQueue->getId(); }

MessageQueueId_t CfdpRouter::getReportReceptionQueue(uint8_t virtualChannel) {
  return tmQueue.getId();
}

ReturnValue_t CfdpRouter::performOperation(uint8_t opCode) {
  lastTcError = TcDistributorBase::performOperation(opCode);
  lastTmError = performTmHandling();
  if (lastTcError != HasReturnvaluesIF::RETURN_OK or lastTmError != HasReturnvaluesIF::RETURN_OK) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CfdpRouter::performTmHandling() {
  ReturnValue_t status;
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  for (status = tmQueue.receiveMessage(&currentMessage); status == RETURN_OK;
       status = tmQueue.receiveMessage(&currentMessage)) {
    ReturnValue_t packetResult = handlePacket();
    if (packetResult != HasReturnvaluesIF::RETURN_OK) {
      result = packetResult;
      triggerEvent(tmtcdistrib::HANDLE_PACKET_FAILED, packetResult, ccsds::PacketType::TM);
    }
  }
  if (status == MessageQueueIF::EMPTY) {
    return result;
  }
  return result;
}
