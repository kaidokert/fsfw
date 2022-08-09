#include "DestHandler.h"

#include <utility>

cfdp::DestHandler::DestHandler(LocalEntityCfg cfg, UserBase& user)
    : cfg(std::move(cfg)), user(user) {}
