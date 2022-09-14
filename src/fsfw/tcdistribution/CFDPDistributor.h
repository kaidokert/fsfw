#ifndef FSFW_TCDISTRIBUTION_CFDPDISTRIBUTOR_H_
#define FSFW_TCDISTRIBUTION_CFDPDISTRIBUTOR_H_

#include <fsfw/tcdistribution/CfdpPacketChecker.h>

#include "../returnvalues/returnvalue.h"
#include "../tmtcpacket/cfdp/CfdpPacketStored.h"
#include "../tmtcservices/AcceptsTelecommandsIF.h"
#include "../tmtcservices/VerificationReporter.h"
#include "CFDPDistributorIF.h"
#include "TcDistributor.h"

/**
 * This class accepts CFDP Telecommands and forwards them to Application
 * services.
 * @ingroup tc_distribution
 */
class CFDPDistributor : public TcDistributor,
                        public CFDPDistributorIF,
                        public AcceptsTelecommandsIF {
 public:
  /**
   * The ctor passes @c set_apid to the checker class and calls the
   * TcDistribution ctor with a certain object id.
   * @param setApid The APID of this receiving Application.
   * @param setObjectId Object ID of the distributor itself
   * @param setPacketSource Object ID of the source of TC packets.
   * Must implement CCSDSDistributorIF.
   */
  CFDPDistributor(uint16_t setApid, object_id_t setObjectId, object_id_t setPacketSource);
  /**
   * The destructor is empty.
   */
  ~CFDPDistributor() override;
  ReturnValue_t registerHandler(AcceptsTelecommandsIF* handler) override;
  MessageQueueId_t getRequestQueue() const override;
  ReturnValue_t initialize() override;
  uint32_t getIdentifier() const override;

 protected:
  uint16_t apid;
  /**
   * The currently handled packet is stored here.
   */
  CfdpPacketStored* currentPacket = nullptr;
  CfdpPacketChecker checker;
  /**
   * With this variable, the current check status is stored to generate
   * acceptance messages later.
   */
  ReturnValue_t tcStatus;

  const object_id_t packetSource;

  /**
   * This method reads the packet service, checks if such a service is
   * registered and forwards the packet to the destination.
   * It also initiates the formal packet check and sending of verification
   * messages.
   * @return Iterator to map entry of found service id
   * or iterator to @c map.end().
   */
  TcMqMapIter selectDestination() override;
  /**
   * The callback here handles the generation of acceptance
   * success/failure messages.
   */
  // ReturnValue_t callbackAfterSending(ReturnValue_t queueStatus) override;
};

#endif /* FSFW_TCDISTRIBUTION_CFDPDISTRIBUTOR_H_ */
