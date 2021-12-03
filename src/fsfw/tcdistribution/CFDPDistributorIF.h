#ifndef FSFW_TCDISTRIBUTION_CFDPDISTRIBUTORIF_H_
#define FSFW_TCDISTRIBUTION_CFDPDISTRIBUTORIF_H_

#include "../tmtcservices/AcceptsTelecommandsIF.h"
#include "../ipc/MessageQueueSenderIF.h"

/**
 * This interface allows CFDP Services to register themselves at a CFDP Distributor.
 * @ingroup tc_distribution
 */
class CFDPDistributorIF {
public:
	/**
	 * The empty virtual destructor.
	 */
	virtual ~CFDPDistributorIF() {
	}
	/**
	 * With this method, Handlers can register themselves at the CFDP Distributor.
	 * @param handler A pointer to the registering Handler.
	 * @return	- @c RETURN_OK on success,
	 * 			- @c RETURN_FAILED on failure.
	 */
	virtual ReturnValue_t registerHandler(AcceptsTelecommandsIF* handler) = 0;
};

#endif /* FSFW_TCDISTRIBUTION_CFDPDISTRIBUTORIF_H_ */
