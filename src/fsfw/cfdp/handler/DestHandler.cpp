#include "DestHandler.h"

#include <utility>

cfdp::DestHandler::DestHandler(LocalEntityCfg cfg, UserBase& user,
                               RemoteConfigTableIF& remoteCfgTable)
    : cfg(std::move(cfg)), user(user), remoteCfgTable(remoteCfgTable) {}
