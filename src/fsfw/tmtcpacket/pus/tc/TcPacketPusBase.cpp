#include "TcPacketPusBase.h"

#include <cstring>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serviceinterface.h"

TcPacketPusBase::TcPacketPusBase(const uint8_t* setData) : SpacePacketBase(setData) {}

TcPacketPusBase::~TcPacketPusBase() {}

void TcPacketPusBase::print() {
  FSFW_LOGI("TcPacketBase::print:\n");
  arrayprinter::print(getWholeData(), getFullSize());
}
