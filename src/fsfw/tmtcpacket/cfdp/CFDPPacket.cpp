#include "fsfw/tmtcpacket/cfdp/CfdpPacket.h"

#include <cstring>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

CfdpReader::CfdpReader(const uint8_t* setData, size_t maxSize)
    : SpacePacketReader(setData, maxSize) {}

CfdpReader::~CfdpReader() = default;

void CfdpReader::print() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::info << "CFDPPacket::print:" << std::endl;
#else
  sif::printInfo("CFDPPacket::print:\n");
#endif
  // arrayprinter::print(getWholeData(), getFullSize());
}
