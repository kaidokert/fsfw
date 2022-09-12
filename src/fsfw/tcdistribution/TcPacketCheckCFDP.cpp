#include "fsfw/tcdistribution/CfdpPacketChecker.h"
#include "fsfw/tmtcpacket/cfdp/CfdpPacketStored.h"

CfdpPacketChecker::CfdpPacketChecker(uint16_t setApid) : apid(setApid) {}

ReturnValue_t CfdpPacketChecker::checkPacket(const SpacePacketReader& currentPacket,
                                             size_t packetLen) {
  return returnvalue::OK;
}

uint16_t CfdpPacketChecker::getApid() const { return apid; }
