#ifndef FRAMEWORK_TCDISTRIBUTION_CCSDSDISTRIBUTOR_H_
#define FRAMEWORK_TCDISTRIBUTION_CCSDSDISTRIBUTOR_H_

#include <map>

#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/tcdistribution/CcsdsDistributorIF.h"
#include "fsfw/tcdistribution/CcsdsPacketChecker.h"
#include "fsfw/tcdistribution/TcDistributorBase.h"
#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"

/**
 * @brief 	An instantiation of the CcsdsDistributorIF.
 * @details
 * It receives Space Packets, and selects a destination depending on the
 * APID of the telecommands.
 * The Secondary Header (with Service/Subservice) is ignored.
 * @ingroup tc_distribution
 */
class CcsdsDistributor : public TcDistributorBase,
                         public CcsdsDistributorIF,
                         public AcceptsTelecommandsIF {
 public:
  /**
   * @brief	The constructor sets the default APID and calls the
   * 			TcDistributor ctor with a certain object id.
   * @details
   * @c tcStore is set in the @c initialize method.
   * @param unknownApid The default APID, where packets with unknown
   * destination are sent to.
   */
  CcsdsDistributor(uint16_t unknownApid, object_id_t setObjectId,
                   StorageManagerIF* tcStore = nullptr, MessageQueueIF* msgQueue = nullptr,
                   CcsdsPacketCheckIF* packetChecker = nullptr);
  /**
   * The destructor is empty.
   */
  ~CcsdsDistributor() override;

  [[nodiscard]] MessageQueueId_t getRequestQueue() const override;
  ReturnValue_t registerApplication(DestInfo info) override;
  [[nodiscard]] uint32_t getIdentifier() const override;
  ReturnValue_t initialize() override;
  [[nodiscard]] const char* getName() const override;

 protected:
  using CcsdsReceiverMap = std::map<uint16_t, DestInfo>;
  CcsdsReceiverMap receiverMap;

  /**
   * This implementation checks if an application with fitting APID has
   * registered and forwards the packet to the according message queue.
   * If the packet is not found, it returns the queue to @c defaultApid,
   * where a Acceptance Failure message should be generated.
   * @return
   *  - @c RETURN_OK if a valid desintation was found, error code otherwise
   *  - @c SerializeIF::STREAM_TOO_SHORT: Packet too short to be a space packet
   */
  ReturnValue_t selectDestination(MessageQueueId_t& destId) override;
  /**
   * The callback here handles the generation of acceptance
   * success/failure messages.
   */
  ReturnValue_t callbackAfterSending(ReturnValue_t queueStatus) override;

  static void handlePacketCheckFailure(ReturnValue_t result);

  ReturnValue_t handleCcsdsHeaderRemoval(ConstStorageAccessor& accessor);
  void print();
  /**
   * The default APID, where packets with unknown APID are sent to.
   */
  uint16_t defaultApid;
  /**
   * A reference to the TC storage must be maintained, as this class handles
   * pure Space Packets and there exists no SpacePacketStored class.
   */
  StorageManagerIF* tcStore = nullptr;

  bool ownedPacketChecker = false;
  CcsdsPacketCheckIF* packetChecker = nullptr;
};

#endif /* FRAMEWORK_TCDISTRIBUTION_CCSDSDISTRIBUTOR_H_ */
