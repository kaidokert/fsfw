#ifndef FSFW_TCDISTRIBUTION_PUSDISTRIBUTOR_H_
#define FSFW_TCDISTRIBUTION_PUSDISTRIBUTOR_H_

#include "PUSDistributorIF.h"
#include "PusPacketChecker.h"
#include "TcDistributor.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include "fsfw/tmtcpacket/pus/tc.h"
#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"
#include "fsfw/tmtcservices/VerificationReporter.h"

class PacketStorageHelper;

/**
 * This class accepts PUS Telecommands and forwards them to Application
 * services. In addition, the class performs a formal packet check and
 * sends acceptance success or failure messages.
 * @ingroup tc_distribution
 */
class PusDistributor : public TcDistributor, public PUSDistributorIF, public AcceptsTelecommandsIF {
 public:
  /**
   * The ctor passes @c set_apid to the checker class and calls the
   * TcDistribution ctor with a certain object id.
   * @param setApid The APID of this receiving Application.
   * @param setObjectId Object ID of the distributor itself
   * @param setPacketSource Object ID of the source of TC packets.
   * Must implement CCSDSDistributorIF.
   */
  PusDistributor(uint16_t setApid, object_id_t setObjectId, object_id_t setPacketSource,
                 StorageManagerIF* store = nullptr);
  /**
   * The destructor is empty.
   */
  ~PusDistributor() override;
  ReturnValue_t registerService(AcceptsTelecommandsIF* service) override;
  MessageQueueId_t getRequestQueue() override;
  ReturnValue_t initialize() override;
  uint16_t getIdentifier() override;

 protected:
  StorageManagerIF* store;
  /**
   * This attribute contains the class, that performs a formal packet check.
   */
  PusPacketChecker checker;
  /**
   * With this class, verification messages are sent to the
   * TC Verification service.
   */
  VerificationReporterIF* verifyChannel = nullptr;
  /**
   * The currently handled packet is stored here.
   */
  PacketStorageHelper* currentPacket = nullptr;
  PusTcReader reader;

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
  ReturnValue_t callbackAfterSending(ReturnValue_t queueStatus) override;
};

#endif /* FSFW_TCDISTRIBUTION_PUSDISTRIBUTOR_H_ */
