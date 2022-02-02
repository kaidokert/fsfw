#include "fsfw/tcdistribution/TcPacketCheckCFDP.h"
#include "fsfw/tmtcpacket/cfdp/CFDPPacketStored.h"

TcPacketCheckCFDP::TcPacketCheckCFDP(uint16_t setApid): apid(setApid) {
}

ReturnValue_t TcPacketCheckCFDP::checkPacket(SpacePacketBase* currentPacket) {
    return RETURN_OK;
}

uint16_t TcPacketCheckCFDP::getApid() const {
    return apid;
}
