#ifndef FSFW_TCDISTRIBUTION_CFDPDISTRIBUTORIF_H_
#define FSFW_TCDISTRIBUTION_CFDPDISTRIBUTORIF_H_

#include "../ipc/MessageQueueSenderIF.h"
#include "../tmtcservices/AcceptsTelecommandsIF.h"

/**
 * This interface allows CFDP Services to register themselves at a CFDP Distributor.
 * @ingroup tc_distribution
 */
class CFDPDistributorIF {
 public:
  /**
   * The empty virtual destructor.
   */
  virtual ~CFDPDistributorIF() = default;
  /**
   * With this method, Handlers can register themselves at the CFDP Distributor.
   * @param handler A pointer to the registering Handler.
   * @return	- @c returnvalue::OK on success,
   * 		- @c returnvalue::FAILED on failure.
   */
  virtual ReturnValue_t registerHandler(AcceptsTelecommandsIF* handler) = 0;
};

#endif /* FSFW_TCDISTRIBUTION_CFDPDISTRIBUTORIF_H_ */
