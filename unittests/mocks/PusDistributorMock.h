#ifndef FSFW_TESTS_PUSDISTRIBUTORMOCK_H
#define FSFW_TESTS_PUSDISTRIBUTORMOCK_H

#include <vector>

#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tcdistribution/PusDistributorIF.h"

class PusDistributorMock : public SystemObject, public PusDistributorIF {
 public:
  PusDistributorMock();
  explicit PusDistributorMock(object_id_t registeredId);
  unsigned int registerCallCount = 0;
  std::vector<const AcceptsTelecommandsIF*> registeredServies;
  ReturnValue_t registerService(const AcceptsTelecommandsIF& service) override;
};

#endif  // FSFW_TESTS_PUSDISTRIBUTORMOCK_H
