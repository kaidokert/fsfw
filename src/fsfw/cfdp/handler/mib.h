#ifndef FSFW_CFDP_MIB_H
#define FSFW_CFDP_MIB_H

#include <utility>

#include "fsfw/cfdp/pdu/PduConfig.h"
#include "FaultHandlerBase.h"

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

}  // namespace cfdp

#endif  // FSFW_CFDP_MIB_H
