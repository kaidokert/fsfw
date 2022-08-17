#include "RemoteConfigTableMock.h"

void cfdp::RemoteConfigTableMock::addRemoteConfig(const cfdp::RemoteEntityCfg& cfg) {
  remoteCfgTable.emplace(cfg.remoteId, cfg);
}

bool cfdp::RemoteConfigTableMock::getRemoteCfg(const cfdp::EntityId& remoteId,
                                               cfdp::RemoteEntityCfg** cfg) {
  auto iter = remoteCfgTable.find(remoteId);
  if (iter == remoteCfgTable.end()) {
    return false;
  }
  *cfg = &iter->second;
  return true;
}
