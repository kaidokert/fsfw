#ifndef FSFW_TCDISTRIBUTION_PUSDISTRIBUTOR_H_
#define FSFW_TCDISTRIBUTION_PUSDISTRIBUTOR_H_

#include <map>

#include "PusDistributorIF.h"
#include "PusPacketChecker.h"
#include "TcDistributorBase.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/tmtcpacket/pus/tc.h"
#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"
#include "fsfw/tmtcservices/VerificationReporter.h"

class CcsdsDistributorIF;

/**
 * This class accepts PUS Telecommands and forwards them to Application
 * services. In addition, the class performs a formal packet check and
 * sends acceptance success or failure messages.
 * @ingroup tc_distribution
 */
class PusDistributor : public TcDistributorBase,
                       public PusDistributorIF,
                       public AcceptsTelecommandsIF {
 public:
  /**
   * The ctor passes @c set_apid to the checker class and calls the
   * TcDistribution ctor with a certain object id.
   * @param setApid The APID of this receiving Application.
   * @param setObjectId Object ID of the distributor itself
   * @param setPacketSource Object ID of the source of TC packets.
   * Must implement CcsdsDistributorIF.
   */
  PusDistributor(uint16_t setApid, object_id_t setObjectId, CcsdsDistributorIF* packetSource,
                 StorageManagerIF* store = nullptr);
  [[nodiscard]] const char* getName() const override;
  /**
   * The destructor is empty.
   */
  ~PusDistributor() override;
  ReturnValue_t registerService(const AcceptsTelecommandsIF& service) override;
  [[nodiscard]] MessageQueueId_t getRequestQueue() const override;
  ReturnValue_t initialize() override;
  [[nodiscard]] uint32_t getIdentifier() const override;

 protected:
  struct ServiceInfo {
    ServiceInfo(const char* name, MessageQueueId_t destId) : name(name), destId(destId) {}

    const char* name;
    MessageQueueId_t destId;
  };
  /// PUS recipient map. The key value will generally be the PUS Service
  using PusReceiverMap = std::map<uint8_t, ServiceInfo>;

  PusReceiverMap receiverMap;
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
  //! Cached for initialization
  CcsdsDistributorIF* ccsdsDistributor = nullptr;
  PusTcReader reader;

  /**
   * With this variable, the current check status is stored to generate
   * acceptance messages later.
   */
  ReturnValue_t tcStatus;

  /**
   * This method reads the packet service, checks if such a service is
   * registered and forwards the packet to the destination.
   * It also initiates the formal packet check and sending of verification
   * messages.
   * @return Iterator to map entry of found service id
   * or iterator to @c map.end().
   */
  ReturnValue_t selectDestination(MessageQueueId_t& destId) override;
  /**
   * The callback here handles the generation of acceptance
   * success/failure messages.
   */
  ReturnValue_t callbackAfterSending(ReturnValue_t queueStatus) override;

  void checkerFailurePrinter() const;
};

#endif /* FSFW_TCDISTRIBUTION_PUSDISTRIBUTOR_H_ */
