#include "fsfw/tmtcpacket/cfdp/CFDPPacket.h"

#include <cstring>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serviceinterface.h"

CFDPPacket::CFDPPacket(const uint8_t* setData) : SpacePacketBase(setData) {}

CFDPPacket::~CFDPPacket() {}

void CFDPPacket::print() {
  FSFW_LOGI("{}", "CFDPPacket::print:\n");
  arrayprinter::print(getWholeData(), getFullSize());
}
