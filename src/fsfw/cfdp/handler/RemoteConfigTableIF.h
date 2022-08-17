#ifndef FSFW_CFDP_HANDLER_REMOTECONFIGTABLEIF_H
#define FSFW_CFDP_HANDLER_REMOTECONFIGTABLEIF_H

#include "fsfw/cfdp/handler/mib.h"

namespace cfdp {

class RemoteConfigTableIF {
 public:
  virtual ~RemoteConfigTableIF() = default;
  virtual bool getRemoteCfg(const cfdp::EntityId& remoteId, cfdp::RemoteEntityCfg** cfg) = 0;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_HANDLER_REMOTECONFIGTABLEIF_H
