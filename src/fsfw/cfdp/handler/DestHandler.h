#ifndef FSFW_CFDP_CFDPDESTHANDLER_H
#define FSFW_CFDP_CFDPDESTHANDLER_H

#include "fsfw/cfdp/handler/mib.h"
#include "fsfw/cfdp/pdu/PduConfig.h"

namespace cfdp {

class DestHandler {
 public:
  DestHandler(LocalEntityCfg cfg /*, CfdpUserBase& user, RemoteConfigTableIF& remoteConfigTable*/);

 private:
  LocalEntityCfg cfg;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_CFDPDESTHANDLER_H
