#include "fsfw/tcdistribution/TcPacketCheckCFDP.h"

#include "fsfw/tmtcpacket/cfdp/CFDPPacketStored.h"

TcPacketCheckCFDP::TcPacketCheckCFDP(uint16_t setApid) : apid(setApid) {}

ReturnValue_t TcPacketCheckCFDP::checkPacket(SpacePacketReader* currentPacket) { return RETURN_OK; }

uint16_t TcPacketCheckCFDP::getApid() const { return apid; }
