#ifndef FSFW_CFDP_HANDLER_REMOTECONFIGTABLEIF_H
#define FSFW_CFDP_HANDLER_REMOTECONFIGTABLEIF_H

#include "fsfw/cfdp/handler/mib.h"

namespace cfdp {

class RemoteConfigTableIF {
 public:
  virtual ~RemoteConfigTableIF() = default;
  virtual bool getRemoteCfg(const cfdp::EntityId& remoteId, cfdp::RemoteEntityCfg** cfg) = 0;
};

/**
 * Helper class for the common case that there is exactly one remote entity
 */
class OneRemoteConfigProvider : public RemoteConfigTableIF {
 public:
  explicit OneRemoteConfigProvider(RemoteEntityCfg cfg) : cfg(std::move(cfg)) {}

  bool getRemoteCfg(const EntityId& remoteId, cfdp::RemoteEntityCfg** cfg_) override {
    if (remoteId != cfg.remoteId) {
      return false;
    }
    *cfg_ = &cfg;
    return true;
  }

 private:
  RemoteEntityCfg cfg;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_HANDLER_REMOTECONFIGTABLEIF_H
