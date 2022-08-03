#include "CfdpDistributor.h"

#include "fsfw/tcdistribution/definitions.h"

CfdpDistributor::CfdpDistributor(CfdpRouterCfg cfg) : TcDistributorBase(cfg.objectId), cfg(cfg) {}

ReturnValue_t CfdpDistributor::registerTcDestination(const cfdp::EntityId& address,
                                                     AcceptsTelecommandsIF& tcDest) {
  for (const auto& dest : tcDestinations) {
    if (dest.id == address) {
      return HasReturnvaluesIF::RETURN_FAILED;
    }
  }
  tcDestinations.emplace_back(address, tcDest.getName(), tcDest.getRequestQueue());
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CfdpDistributor::selectDestination(MessageQueueId_t& destId) {
  auto accessorPair = cfg.tcStore.getData(currentMessage.getStorageId());
  if (accessorPair.first != HasReturnvaluesIF::RETURN_OK) {
    return accessorPair.first;
  }
  ReturnValue_t result = pduReader.setData(accessorPair.second.data(), accessorPair.second.size());
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = pduReader.parseData();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  cfdp::EntityId foundId;
  pduReader.getDestId(foundId);
  bool destFound = false;
  for (const auto& dest : tcDestinations) {
    if (dest.id == foundId) {
      destId = dest.queueId;
      destFound = true;
    }
  }
  if (not destFound) {
    // TODO: Warning and event?
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  // Packet was forwarded successfully, so do not delete it.
  accessorPair.second.release();
  return HasReturnvaluesIF::RETURN_OK;
}

const char* CfdpDistributor::getName() const { return "CFDP Router"; }

uint32_t CfdpDistributor::getIdentifier() const { return 0; }

MessageQueueId_t CfdpDistributor::getRequestQueue() const { return tcQueue->getId(); }
