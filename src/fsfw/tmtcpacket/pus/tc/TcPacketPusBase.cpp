#include "TcPacketPusBase.h"

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

#include <cstring>

TcPacketPusBase::TcPacketPusBase(const uint8_t* setData): SpacePacketBase(setData) {}

TcPacketPusBase::~TcPacketPusBase() {}

void TcPacketPusBase::print() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "TcPacketBase::print:" << std::endl;
#else
    sif::printInfo("TcPacketBase::print:\n");
#endif
    arrayprinter::print(getWholeData(), getFullSize());
}
