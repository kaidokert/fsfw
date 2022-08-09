#include "DestHandler.h"

#include <utility>

cfdp::DestHandler::DestHandler(LocalEntityCfg cfg) : cfg(std::move(cfg)) {}
