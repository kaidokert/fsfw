#include "PusDistributorMock.h"

PusDistributorMock::PusDistributorMock() : SystemObject(objects::NO_OBJECT, false) {}

PusDistributorMock::PusDistributorMock(object_id_t registeredId)
    : SystemObject(registeredId, true) {}

ReturnValue_t PusDistributorMock::registerService(const AcceptsTelecommandsIF& service) {
  registerCallCount++;
  registeredServies.push_back(&service);
  return returnvalue::OK;
}
