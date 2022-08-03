#ifndef FSFW_TCDISTRIBUTION_CFDPDISTRIBUTORIF_H_
#define FSFW_TCDISTRIBUTION_CFDPDISTRIBUTORIF_H_

#include "fsfw/cfdp/definitions.h"
#include "fsfw/cfdp/pdu/PduHeaderIF.h"
#include "fsfw/ipc/MessageQueueSenderIF.h"
#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"

/**
 * This interface allows CFDP Services to register themselves at a CFDP Distributor.
 * @ingroup tc_distribution
 */
class CfdpRouterIF {
 public:
  /**
   * The empty virtual destructor.
   */
  virtual ~CfdpRouterIF() = default;

  virtual ReturnValue_t registerTmSink(cfdp::EntityId address, AcceptsTelemetryIF& tmDest) = 0;
  /**
   * With this method, Handlers can register themselves at the CFDP Distributor.
   * @param handler A pointer to the registering Handler.
   * @return	- @c RETURN_OK on success,
   * 			- @c RETURN_FAILED on failure.
   */
  virtual ReturnValue_t registerTcDestination(cfdp::EntityId address,
                                              AcceptsTelecommandsIF& tcDest) = 0;
};

#endif /* FSFW_TCDISTRIBUTION_CFDPDISTRIBUTORIF_H_ */
