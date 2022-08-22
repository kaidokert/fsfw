#ifndef FSFW_TCDISTRIBUTION_TCPACKETCHECKPUS_H_
#define FSFW_TCDISTRIBUTION_TCPACKETCHECKPUS_H_

#include "fsfw/FSFW.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/tmtcpacket/pus/defs.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"

/**
 * This class performs a formal packet check for incoming PUS Telecommand Packets.
 * Currently, it only checks if the APID and CRC are correct.
 * @ingroup tc_distribution
 */
class PusPacketChecker {
 public:
  /**
   * The constructor only sets the APID attribute.
   * @param set_apid The APID to set.
   */
  explicit PusPacketChecker(uint16_t apid, ccsds::PacketType packetType,
                            ecss::PusVersion = ecss::PusVersion::PUS_C);

  ReturnValue_t checkPacket(const PusTcReader& currentPacket, size_t packetLen);

  [[nodiscard]] uint16_t getApid() const;

 protected:
  ecss::PusVersion pusVersion;
  uint16_t apid;

 private:
};

#endif /* FSFW_TCDISTRIBUTION_TCPACKETCHECKPUS_H_ */
