#ifndef FSFW_TESTS_PUSDISTRIBUTORMOCK_H
#define FSFW_TESTS_PUSDISTRIBUTORMOCK_H

#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tcdistribution/PusDistributorIF.h"

class PusDistributorMock : public SystemObject, public PusDistributorIF {
 public:
  PusDistributorMock();
  explicit PusDistributorMock(object_id_t registeredId);
  unsigned int registerCallCount = 0;
  AcceptsTelecommandsIF* lastServiceArg = nullptr;
  ReturnValue_t registerService(AcceptsTelecommandsIF* service) override;
};

#endif  // FSFW_TESTS_PUSDISTRIBUTORMOCK_H
