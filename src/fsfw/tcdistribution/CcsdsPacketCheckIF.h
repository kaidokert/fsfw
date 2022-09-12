#ifndef FSFW_TCDISTRIBUTION_TCPACKETCHECKIF_H_
#define FSFW_TCDISTRIBUTION_TCPACKETCHECKIF_H_

#include <cstddef>

#include "fsfw/returnvalues/returnvalue.h"

class SpacePacketReader;

/**
 * This interface is used by PacketCheckers for PUS packets and CFDP packets .
 * @ingroup tc_distribution
 */
class CcsdsPacketCheckIF {
 public:
  /**
   * The empty virtual destructor.
   */
  virtual ~CcsdsPacketCheckIF() = default;

  /**
   * This is the actual method to formally check a certain Packet.
   * The packet's Application Data can not be checked here.
   * @param current_packet The packet to check
   * @return	- @c returnvalue::OK on success.
   * 		- @c INCORRECT_CHECKSUM if checksum is invalid.
   * 		- @c ILLEGAL_APID if APID does not match.
   */
  virtual ReturnValue_t checkPacket(const SpacePacketReader& currentPacket, size_t packetLen) = 0;
};

#endif /* FSFW_TCDISTRIBUTION_TCPACKETCHECKIF_H_ */
