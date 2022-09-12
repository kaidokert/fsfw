#ifndef FSFW_TCDISTRIBUTION_TCPACKETCHECKCFDP_H_
#define FSFW_TCDISTRIBUTION_TCPACKETCHECKCFDP_H_

#include "CcsdsPacketCheckIF.h"
#include "fsfw/FSFW.h"

class CfdpPacketStored;

/**
 * This class performs a formal packet check for incoming CFDP Packets.
 * @ingroup tc_distribution
 */
class CfdpPacketChecker : public CcsdsPacketCheckIF {
 protected:
  /**
   * The packet id each correct packet should have.
   * It is composed of the APID and some static fields.
   */
  uint16_t apid;

 public:
  /**
   * The constructor only sets the APID attribute.
   * @param set_apid The APID to set.
   */
  explicit CfdpPacketChecker(uint16_t setApid);

  ReturnValue_t checkPacket(const SpacePacketReader& currentPacket, size_t packetLen) override;

  [[nodiscard]] uint16_t getApid() const;
};

#endif /* FSFW_TCDISTRIBUTION_TCPACKETCHECKCFDP_H_ */
