#ifndef FSFW_TCDISTRIBUTION_TCPACKETCHECKPUS_H_
#define FSFW_TCDISTRIBUTION_TCPACKETCHECKPUS_H_

#include "CcsdsPacketCheckIF.h"
#include "CcsdsPacketCheckerBase.h"
#include "fsfw/FSFW.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include "fsfw/tmtcpacket/pus/defs.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"

class PacketCheckIF : public ReadablePacketIF, public PusIF {};

/**
 * This class performs a formal packet check for incoming PUS Telecommand Packets.
 * Currently, it only checks if the APID and CRC are correct.
 * @ingroup tc_distribution
 */
class PusPacketChecker : public CcsdsPacketCheckerBase {
 public:
  /**
   * The constructor only sets the APID attribute.
   * @param set_apid The APID to set.
   */
  PusPacketChecker(uint16_t setApid, ccsds::PacketType packetType,
                   ecss::PusVersion = ecss::PusVersion::PUS_C);

  ReturnValue_t checkPacket(PacketCheckIF* currentPacket, size_t packetLen);

  [[nodiscard]] uint16_t getApid() const;

 protected:
  ecss::PusVersion pusVersion;

 private:
  ReturnValue_t checkPacket(SpacePacketReader* currentPacket, size_t packetLen) override;
};

#endif /* FSFW_TCDISTRIBUTION_TCPACKETCHECKPUS_H_ */
