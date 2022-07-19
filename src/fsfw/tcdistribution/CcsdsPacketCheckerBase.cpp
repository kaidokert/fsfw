#include "CcsdsPacketCheckerBase.h"

#include "fsfw/tcdistribution/definitions.h"
#include "fsfw/tmtcpacket/SpacePacketReader.h"

CcsdsPacketCheckerBase::CcsdsPacketCheckerBase(uint16_t apid, ccsds::PacketType packetType_,
                                               uint8_t ccsdsVersion_)
    : apid(apid), packetType(packetType_), ccsdsVersion(ccsdsVersion_) {}

ReturnValue_t CcsdsPacketCheckerBase::checkPacket(SpacePacketReader* currentPacket,
                                                  size_t packetLen) {
  if (currentPacket->getApid() != apid) {
    return tcdistrib::INVALID_APID;
  }
  if (currentPacket->getVersion() != ccsdsVersion) {
    return tcdistrib::INVALID_CCSDS_VERSION;
  }
  if (currentPacket->getPacketType() != packetType) {
    return tcdistrib::INVALID_PACKET_TYPE;
  }
  return HasReturnvaluesIF::RETURN_OK;
}