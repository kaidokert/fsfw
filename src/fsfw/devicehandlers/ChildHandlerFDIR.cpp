#include "fsfw/devicehandlers/ChildHandlerFDIR.h"

ChildHandlerFDIR::ChildHandlerFDIR(object_id_t owner, object_id_t faultTreeParent,
                                   uint32_t recoveryCount)
    : DeviceHandlerFailureIsolation(owner, faultTreeParent) {
  recoveryCounter.setFailureThreshold(recoveryCount);
}

ChildHandlerFDIR::~ChildHandlerFDIR() {}
