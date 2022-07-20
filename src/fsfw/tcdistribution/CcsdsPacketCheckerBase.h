#ifndef FSFW_TESTS_CCSDSPACKETCHECKERBASE_H
#define FSFW_TESTS_CCSDSPACKETCHECKERBASE_H

#include "CcsdsPacketCheckIF.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketIF.h"

class CcsdsPacketCheckerBase : public CcsdsPacketCheckIF, public HasReturnvaluesIF {
 public:
  CcsdsPacketCheckerBase(uint16_t apid, ccsds::PacketType packetType, uint8_t ccsdsVersion = 0b000);
  ReturnValue_t checkPacket(SpacePacketReader* currentPacket, size_t packetLen) override;

 protected:
  uint16_t apid;
  ccsds::PacketType packetType;
  uint8_t ccsdsVersion;
};
#endif  // FSFW_TESTS_CCSDSPACKETCHECKERBASE_H
