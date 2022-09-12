#include "PusDistributorMock.h"

PusDistributorMock::PusDistributorMock() : SystemObject(objects::NO_OBJECT, false) {}

PusDistributorMock::PusDistributorMock(object_id_t registeredId)
    : SystemObject(registeredId, true) {}

ReturnValue_t PusDistributorMock::registerService(AcceptsTelecommandsIF *service) {
  registerCallCount++;
  lastServiceArg = service;
  return returnvalue::OK;
}
