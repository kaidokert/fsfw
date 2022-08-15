#ifndef FSFW_TESTS_CCSDSPACKETCHECKERBASE_H
#define FSFW_TESTS_CCSDSPACKETCHECKERBASE_H

#include "CcsdsPacketCheckIF.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketIF.h"

class CcsdsPacketChecker : public CcsdsPacketCheckIF {
 public:
  CcsdsPacketChecker(ccsds::PacketType packetType, uint8_t ccsdsVersion = 0b000);

  void setApidToCheck(uint16_t apid);
  ReturnValue_t checkPacket(const SpacePacketReader& currentPacket, size_t packetLen) override;

 protected:
  bool checkApid = false;
  uint16_t apid = 0;
  ccsds::PacketType packetType;
  uint8_t ccsdsVersion;
};
#endif  // FSFW_TESTS_CCSDSPACKETCHECKERBASE_H
