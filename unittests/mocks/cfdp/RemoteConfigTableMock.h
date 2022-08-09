#ifndef FSFW_TESTS_CFDP_REMOTCONFIGTABLEMOCK_H
#define FSFW_TESTS_CFDP_REMOTCONFIGTABLEMOCK_H

#include "fsfw/cfdp/handler/RemoteConfigTableIF.h"

namespace cfdp {

class RemoteConfigTableMock: public RemoteConfigTableIF {
 public:
  bool getRemoteCfg(EntityId remoteId, RemoteEntityCfg *cfg) override;
};

}

#endif  // FSFW_TESTS_CFDP_REMOTCONFIGTABLEMOCK_H
