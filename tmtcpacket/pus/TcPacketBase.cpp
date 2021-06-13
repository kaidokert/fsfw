#include "TcPacketBase.h"

#include "../../globalfunctions/CRC.h"
#include "../../globalfunctions/arrayprinter.h"
#include "../../serviceinterface/ServiceInterface.h"

#include <cstring>

TcPacketBase::TcPacketBase(const uint8_t* setData): SpacePacketBase(setData) {}

TcPacketBase::~TcPacketBase() {}

void TcPacketBase::print() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "TcPacketBase::print:" << std::endl;
#else
    sif::printInfo("TcPacketBase::print:\n");
#endif
    arrayprinter::print(getWholeData(), getFullSize());
}
