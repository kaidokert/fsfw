#include "fsfw/tmtcpacket/cfdp/CFDPPacket.h"

#include <cstring>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

CFDPPacket::CFDPPacket(const uint8_t* setData) : SpacePacketBase(setData) {}

CFDPPacket::~CFDPPacket() {}

void CFDPPacket::print() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::info << "CFDPPacket::print:" << std::endl;
#else
  sif::printInfo("CFDPPacket::print:\n");
#endif
  arrayprinter::print(getWholeData(), getFullSize());
}
