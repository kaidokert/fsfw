#include "PusTmCreator.h"

PusTmCreator::PusTmCreator(PusTmParams params_) : params(params_){};

uint16_t PusTmCreator::getPacketId() const { return 0; }
uint16_t PusTmCreator::getPacketSeqCtrl() const { return 0; }
uint16_t PusTmCreator::getPacketDataLen() const { return 0; }
uint8_t PusTmCreator::getPusVersion() const { return 0; }
uint8_t PusTmCreator::getService() const { return 0; }
uint8_t PusTmCreator::getSubService() const { return 0; }
