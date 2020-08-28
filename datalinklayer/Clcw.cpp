/**
 * @file	Clcw.cpp
 * @brief	This file defines the Clcw class.
 * @date	17.04.2013
 * @author	baetz
 */



#include "Clcw.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

Clcw::Clcw() {
	content.raw = 0;
	content.status = STATUS_FIELD_DEFAULT;
}

Clcw::~Clcw() {
}

void Clcw::setVirtualChannel(uint8_t setChannel) {
	content.virtualChannelIdSpare = ((setChannel & 0x3F) << 2);
}

void Clcw::setLockoutFlag(bool lockout) {
	content.flags = (content.flags & LOCKOUT_FLAG_MASK) | (lockout << LOCKOUT_FLAG_POSITION);
}

void Clcw::setWaitFlag(bool waitFlag) {
	content.flags = (content.flags & WAIT_FLAG_MASK) | (waitFlag << WAIT_FLAG_POSITION);
}

void Clcw::setRetransmitFlag(bool retransmitFlag) {
	content.flags = (content.flags & RETRANSMIT_FLAG_MASK) | (retransmitFlag << RETRANSMIT_FLAG_POSITION);
}

void Clcw::setFarmBCount(uint8_t count) {
	content.flags = (content.flags & FARM_B_COUNT_MASK) | ((count & 0x03) << 1);
}

void Clcw::setReceiverFrameSequenceNumber(uint8_t vR) {
	content.vRValue = vR;
}

uint32_t Clcw::getAsWhole() {
	return content.raw;
}

void Clcw::setRFAvailable(bool rfAvailable) {
	content.flags = (content.flags & NO_RF_AVIALABLE_MASK) | (!rfAvailable << NO_RF_AVIALABLE_POSITION);
}

void Clcw::setBitLock(bool bitLock) {
	content.flags = (content.flags & NO_BIT_LOCK_MASK) | (!bitLock << NO_BIT_LOCK_POSITION);
}

void Clcw::print() {
	sif::debug << "Clcw::print: Clcw is: " << std::hex << getAsWhole() << std::dec << std::endl;
}

void Clcw::setWhole(uint32_t rawClcw) {
	content.raw = rawClcw;
}
