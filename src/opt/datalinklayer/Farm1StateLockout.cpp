/**
 * @file	Farm1StateLockout.cpp
 * @brief	This file defines the Farm1StateLockout class.
 * @date	24.04.2013
 * @author	baetz
 */



#include "ClcwIF.h"
#include "Farm1StateLockout.h"
#include "TcTransferFrame.h"
#include "VirtualChannelReception.h"
Farm1StateLockout::Farm1StateLockout(VirtualChannelReception* setMyVC) : myVC(setMyVC) {
}

ReturnValue_t Farm1StateLockout::handleADFrame(TcTransferFrame* frame,
		ClcwIF* clcw) {
	return FARM_IN_LOCKOUT;
}

ReturnValue_t Farm1StateLockout::handleBCUnlockCommand(ClcwIF* clcw) {
	myVC->farmBCounter++;
	clcw->setRetransmitFlag(false);
	clcw->setLockoutFlag( false );
	clcw->setWaitFlag( false );
	myVC->currentState = &(myVC->openState);
	return BC_IS_UNLOCK_COMMAND;
}

ReturnValue_t Farm1StateLockout::handleBCSetVrCommand(ClcwIF* clcw,
		uint8_t vr) {
	myVC->farmBCounter++;
	return BC_IS_SET_VR_COMMAND;
}
