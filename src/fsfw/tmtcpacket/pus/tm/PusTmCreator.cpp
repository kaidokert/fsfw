#include "PusTmCreator.h"

PusTmCreator::PusTmCreator(PusTmParams& params_) : params(params_){};

uint16_t PusTmCreator::getPacketIdRaw() const { return 0; }
uint16_t PusTmCreator::getPacketSeqCtrlRaw() const { return 0; }
uint16_t PusTmCreator::getPacketDataLen() const { return 0; }
uint8_t PusTmCreator::getPusVersion() const { return 0; }
uint8_t PusTmCreator::getService() const { return 0; }
uint8_t PusTmCreator::getSubService() const { return 0; }
const uint8_t* PusTmCreator::getUserData(size_t& appDataLen) const { return nullptr; }
uint16_t PusTmCreator::getUserDataSize() const { return 0; }
