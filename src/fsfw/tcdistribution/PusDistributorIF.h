#ifndef FSFW_TCDISTRIBUTION_PUSDISTRIBUTORIF_H_
#define FSFW_TCDISTRIBUTION_PUSDISTRIBUTORIF_H_

#include "../ipc/MessageQueueSenderIF.h"
#include "../tmtcservices/AcceptsTelecommandsIF.h"

/**
 * This interface allows PUS Services to register themselves at a PUS Distributor.
 * @ingroup tc_distribution
 */
class PusDistributorIF {
 public:
  /**
   * The empty virtual destructor.
   */
  virtual ~PusDistributorIF() = default;
  /**
   * With this method, Services can register themselves at the PUS Distributor.
   * @param service A pointer to the registering Service.
   * @return	- @c returnvalue::OK on success,
   * 		- @c returnvalue::FAILED on failure.
   */
  virtual ReturnValue_t registerService(const AcceptsTelecommandsIF& service) = 0;
};

#endif /* FSFW_TCDISTRIBUTION_PUSDISTRIBUTORIF_H_ */
