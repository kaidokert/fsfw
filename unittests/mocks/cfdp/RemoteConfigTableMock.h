#ifndef FSFW_TESTS_CFDP_REMOTCONFIGTABLEMOCK_H
#define FSFW_TESTS_CFDP_REMOTCONFIGTABLEMOCK_H

#include <map>

#include "fsfw/cfdp/handler/RemoteConfigTableIF.h"

namespace cfdp {

class RemoteConfigTableMock : public RemoteConfigTableIF {
 public:
  void addRemoteConfig(const RemoteEntityCfg& cfg);
  bool getRemoteCfg(const cfdp::EntityId& remoteId, cfdp::RemoteEntityCfg** cfg) override;

  std::map<EntityId, RemoteEntityCfg> remoteCfgTable;
};

}  // namespace cfdp

#endif  // FSFW_TESTS_CFDP_REMOTCONFIGTABLEMOCK_H
