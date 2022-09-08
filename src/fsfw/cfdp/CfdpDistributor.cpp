#include "CfdpDistributor.h"

#include "fsfw/tcdistribution/definitions.h"

CfdpDistributor::CfdpDistributor(CfdpDistribCfg cfg)
    : TcDistributorBase(cfg.objectId, cfg.tcQueue), cfg(cfg) {}

ReturnValue_t CfdpDistributor::registerTcDestination(const cfdp::EntityId& address,
                                                     AcceptsTelecommandsIF& tcDest) {
  for (const auto& dest : tcDestinations) {
    if (dest.id == address) {
      return returnvalue::FAILED;
    }
  }
  tcDestinations.emplace_back(address, tcDest.getName(), tcDest.getRequestQueue());
  return returnvalue::OK;
}

ReturnValue_t CfdpDistributor::selectDestination(MessageQueueId_t& destId) {
  auto accessorPair = cfg.tcStore.getData(currentMessage.getStorageId());
  if (accessorPair.first != returnvalue::OK) {
    return accessorPair.first;
  }
  ReturnValue_t result =
      pduReader.setReadOnlyData(accessorPair.second.data(), accessorPair.second.size());
  if (result != returnvalue::OK) {
    return result;
  }
  result = pduReader.parseData();
  if (result != returnvalue::OK) {
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
    return tmtcdistrib::NO_DESTINATION_FOUND;
  }
  // Packet was forwarded successfully, so do not delete it.
  accessorPair.second.release();
  return returnvalue::OK;
}

const char* CfdpDistributor::getName() const { return "CFDP Distributor"; }

uint32_t CfdpDistributor::getIdentifier() const { return 0; }

MessageQueueId_t CfdpDistributor::getRequestQueue() const { return tcQueue->getId(); }
