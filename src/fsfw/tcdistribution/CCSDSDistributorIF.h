#ifndef FSFW_TCDISTRIBUTION_CCSDSDISTRIBUTORIF_H_
#define FSFW_TCDISTRIBUTION_CCSDSDISTRIBUTORIF_H_

#include "../ipc/MessageQueueSenderIF.h"
#include "../tmtcservices/AcceptsTelecommandsIF.h"
/**
 * This is the Interface to a CCSDS Distributor.
 * On a CCSDS Distributor, Applications (in terms of CCSDS) may register
 * themselves, either by passing a pointer to themselves (and implementing the
 * CCSDSApplicationIF),  or by explicitly passing an APID and a MessageQueueId
 * to route the TC's to.
 * @ingroup tc_distribution
 */
class CCSDSDistributorIF {
 public:
  /**
   * With this call, a class implementing the CCSDSApplicationIF can register
   * at the distributor.
   * @param application A pointer to the Application to register.
   * @return	- @c returnvalue::OK on success,
   * 		- @c returnvalue::FAILED on failure.
   */
  virtual ReturnValue_t registerApplication(AcceptsTelecommandsIF* application) = 0;
  /**
   * With this call, other Applications can register to the CCSDS distributor.
   * This is done by passing an APID and a MessageQueueId to the method.
   * @param apid	The APID to register.
   * @param id	The MessageQueueId of the message queue to send the
   *              TC Packets to.
   * @return	- @c returnvalue::OK on success,
   * 		- @c returnvalue::FAILED on failure.
   */
  virtual ReturnValue_t registerApplication(uint16_t apid, MessageQueueId_t id) = 0;
  /**
   * The empty virtual destructor.
   */
  virtual ~CCSDSDistributorIF() = default;
};

#endif /* FSFW_TCDISTRIBUTION_CCSDSDISTRIBUTORIF_H_ */
