#ifndef FRAMEWORK_TCDISTRIBUTION_CCSDSDISTRIBUTOR_H_
#define FRAMEWORK_TCDISTRIBUTION_CCSDSDISTRIBUTOR_H_

#include "../objectmanager/ObjectManagerIF.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../tcdistribution/CCSDSDistributorIF.h"
#include "../tcdistribution/TcDistributor.h"
#include "../tmtcservices/AcceptsTelecommandsIF.h"

/**
 * @brief 	An instantiation of the CCSDSDistributorIF.
 * @details
 * It receives Space Packets, and selects a destination depending on the
 * APID of the telecommands.
 * The Secondary Header (with Service/Subservice) is ignored.
 * @ingroup tc_distribution
 */
class CCSDSDistributor : public TcDistributor,
                         public CCSDSDistributorIF,
                         public AcceptsTelecommandsIF {
 public:
  /**
   * @brief	The constructor sets the default APID and calls the
   * 			TcDistributor ctor with a certain object id.
   * @details
   * @c tcStore is set in the @c initialize method.
   * @param setDefaultApid The default APID, where packets with unknown
   * destination are sent to.
   */
  CCSDSDistributor(uint16_t setDefaultApid, object_id_t setObjectId);
  /**
   * The destructor is empty.
   */
  virtual ~CCSDSDistributor();

  MessageQueueId_t getRequestQueue() override;
  ReturnValue_t registerApplication(uint16_t apid, MessageQueueId_t id) override;
  ReturnValue_t registerApplication(AcceptsTelecommandsIF* application) override;
  uint16_t getIdentifier() override;
  ReturnValue_t initialize() override;

 protected:
  /**
   * This implementation checks if an application with fitting APID has
   * registered and forwards the packet to the according message queue.
   * If the packet is not found, it returns the queue to @c defaultApid,
   * where a Acceptance Failure message should be generated.
   * @return Iterator to map entry of found APID or iterator to default APID.
   */
  TcMqMapIter selectDestination() override;
  /**
   * The callback here handles the generation of acceptance
   * success/failure messages.
   */
  ReturnValue_t callbackAfterSending(ReturnValue_t queueStatus) override;

  /**
   * The default APID, where packets with unknown APID are sent to.
   */
  uint16_t defaultApid;
  /**
   * A reference to the TC storage must be maintained, as this class handles
   * pure Space Packets and there exists no SpacePacketStored class.
   */
  StorageManagerIF* tcStore = nullptr;
};

#endif /* FRAMEWORK_TCDISTRIBUTION_CCSDSDISTRIBUTOR_H_ */
