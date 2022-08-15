#include "CcsdsPacketChecker.h"

#include "fsfw/tcdistribution/definitions.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketReader.h"

CcsdsPacketChecker::CcsdsPacketChecker(ccsds::PacketType packetType_, uint8_t ccsdsVersion_)
    : packetType(packetType_), ccsdsVersion(ccsdsVersion_) {}

ReturnValue_t CcsdsPacketChecker::checkPacket(const SpacePacketReader& currentPacket,
                                              size_t packetLen) {
  if (checkApid) {
    if (currentPacket.getApid() != apid) {
      return tcdistrib::INVALID_APID;
    }
  }

  if (currentPacket.getVersion() != ccsdsVersion) {
    return tcdistrib::INVALID_CCSDS_VERSION;
  }
  if (currentPacket.getPacketType() != packetType) {
    return tcdistrib::INVALID_PACKET_TYPE;
  }
  // This assumes that the getFullPacketLen version uses the space packet data length field
  if (currentPacket.getFullPacketLen() != packetLen) {
    return tcdistrib::INCOMPLETE_PACKET;
  }
  return returnvalue::OK;
}

void CcsdsPacketChecker::setApidToCheck(uint16_t apid_) {
  apid = apid_;
  checkApid = true;
}
