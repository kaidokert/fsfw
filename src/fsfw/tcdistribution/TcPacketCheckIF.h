#ifndef FSFW_TCDISTRIBUTION_TCPACKETCHECKIF_H_
#define FSFW_TCDISTRIBUTION_TCPACKETCHECKIF_H_

#include "../returnvalues/HasReturnvaluesIF.h"

class SpacePacketBase;

/**
 * This interface is used by PacketCheckers for PUS packets and CFDP packets .
 * @ingroup tc_distribution
 */
class TcPacketCheckIF {
 public:
  /**
   * The empty virtual destructor.
   */
  virtual ~TcPacketCheckIF() {}

  /**
   * This is the actual method to formally check a certain Packet.
   * The packet's Application Data can not be checked here.
   * @param current_packet The packet to check
   * @return	- @c returnvalue::OK on success.
   * 			- @c INCORRECT_CHECKSUM if checksum is invalid.
   * 			- @c ILLEGAL_APID if APID does not match.
   */
  virtual ReturnValue_t checkPacket(SpacePacketBase* currentPacket) = 0;
};

#endif /* FSFW_TCDISTRIBUTION_TCPACKETCHECKIF_H_ */
