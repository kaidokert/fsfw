#ifndef FSFW_CFDP_CFDPDESTHANDLER_H
#define FSFW_CFDP_CFDPDESTHANDLER_H

#include "RemoteConfigTableIF.h"
#include "UserBase.h"
#include "fsfw/cfdp/handler/mib.h"
#include "fsfw/cfdp/pdu/PduConfig.h"

namespace cfdp {

class DestHandler {
 public:
  DestHandler(LocalEntityCfg cfg, UserBase& user, RemoteConfigTableIF& remoteCfgTable);

 private:
  LocalEntityCfg cfg;
  UserBase& user;
  RemoteConfigTableIF& remoteCfgTable;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_CFDPDESTHANDLER_H
