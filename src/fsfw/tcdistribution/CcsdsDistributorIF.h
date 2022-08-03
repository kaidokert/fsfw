#ifndef FSFW_TCDISTRIBUTION_CCSDSDISTRIBUTORIF_H_
#define FSFW_TCDISTRIBUTION_CCSDSDISTRIBUTORIF_H_

#include "fsfw/ipc/MessageQueueSenderIF.h"
#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"

/**
 * This is the Interface to a CCSDS Distributor.
 * On a CCSDS Distributor, Applications (in terms of CCSDS) may register
 * themselves, either by passing a pointer to themselves (and implementing the
 * CCSDSApplicationIF),  or by explicitly passing an APID and a MessageQueueId
 * to route the TC's to.
 * @ingroup tc_distribution
 */
class CcsdsDistributorIF {
 public:
  struct DestInfo {
    DestInfo(const char* name, uint16_t apid, MessageQueueId_t destId, bool removeHeader)
        : name(name), apid(apid), destId(destId), removeHeader(removeHeader) {}
    DestInfo(const AcceptsTelecommandsIF& ccsdsReceiver, bool removeHeader_)
        : name(ccsdsReceiver.getName()) {
      apid = ccsdsReceiver.getIdentifier();
      destId = ccsdsReceiver.getRequestQueue();
      removeHeader = removeHeader_;
    }
    const char* name;
    uint16_t apid;
    MessageQueueId_t destId;
    bool removeHeader;
  };

  /**
   * With this call, other Applications can register to the CCSDS distributor.
   * This is done by passing an APID and a MessageQueueId to the method.
   * @param info	Contains all necessary info to register an application.
   * @return
   *  - @c RETURN_OK on success,
   *  - @c RETURN_FAILED or tmtcdistrib error code on failure.
   *  - @c tmtcdistrib::INVALID_CCSDS_VERSION
   *  - @c tmtcdistrib::INVALID_APID No APID available to handle this packet
   *  - @c tmtcdistrib::INVALID_PACKET_TYPE Packet type TM detected
   *  - @c tmtcdistrib::INCORRECT_PRIMARY_HEADER Something other wrong with primary header
   *  - @c tmtcdistrib::INCOMPLETE_PACKET Size missmatch between data length field and actual
   *       length
   */
  virtual ReturnValue_t registerApplication(DestInfo info) = 0;

  /**
   * The empty virtual destructor.
   */
  virtual ~CcsdsDistributorIF() = default;
};

#endif /* FSFW_TCDISTRIBUTION_CCSDSDISTRIBUTORIF_H_ */
