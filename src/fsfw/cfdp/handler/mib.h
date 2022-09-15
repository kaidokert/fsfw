#ifndef FSFW_CFDP_MIB_H
#define FSFW_CFDP_MIB_H

#include <utility>

#include "FaultHandlerBase.h"
#include "fsfw/cfdp/pdu/PduConfig.h"

namespace cfdp {

struct IndicationCfg {
  bool eofSentIndicRequired = true;
  bool eofRecvIndicRequired = true;
  bool fileSegmentRecvIndicRequired = true;
  bool transactionFinishedIndicRequired = true;
  bool suspendedIndicRequired = true;
  bool resumedIndicRequired = true;
};

struct LocalEntityCfg {
  LocalEntityCfg(EntityId localId, IndicationCfg indicationCfg, FaultHandlerBase& fhBase)
      : localId(std::move(localId)), indicCfg(indicationCfg), fhBase(fhBase) {}

  EntityId localId;
  IndicationCfg indicCfg;
  FaultHandlerBase& fhBase;
};

struct RemoteEntityCfg {
  explicit RemoteEntityCfg(EntityId id) : remoteId(std::move(id)) {}
  EntityId remoteId;
  size_t maxFileSegmentLen = 2048;
  bool closureRequested = false;
  bool crcOnTransmission = false;
  TransmissionMode defaultTransmissionMode = TransmissionMode::UNACKNOWLEDGED;
  ChecksumType defaultChecksum = ChecksumType::NULL_CHECKSUM;
  const uint8_t version = CFDP_VERSION_2;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_MIB_H
