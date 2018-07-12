/**
 * @file	Farm1StateOpen.cpp
 * @brief	This file defines the Farm1StateOpen class.
 * @date	24.04.2013
 * @author	baetz
 */




#include <framework/datalinklayer/ClcwIF.h>
#include <framework/datalinklayer/Farm1StateOpen.h>
#include <framework/datalinklayer/TcTransferFrame.h>
#include <framework/datalinklayer/VirtualChannelReception.h>

Farm1StateOpen::Farm1StateOpen(VirtualChannelReception* setMyVC) : myVC(setMyVC) {
}

ReturnValue_t Farm1StateOpen::handleADFrame(TcTransferFrame* frame,
		ClcwIF* clcw) {
	int8_t diff = frame->getSequenceNumber() - myVC->vR;
	if (diff == 0 ) {
		myVC->vR++;
		clcw->setRetransmitFlag(false);
		return RETURN_OK;
	} else if (diff < myVC->positiveWindow && diff > 0 ) {
		clcw->setRetransmitFlag(true);
		return NS_POSITIVE_W;
	} else if (diff < 0 && diff >= -myVC->negativeWindow) {
		return NS_NEGATIVE_W;
	} else {
		clcw->setLockoutFlag(true);
		myVC->currentState = &(myVC->lockoutState);
		return NS_LOCKOUT;
	}
}

ReturnValue_t Farm1StateOpen::handleBCUnlockCommand( ClcwIF* clcw ) {
	myVC->farmBCounter++;
	clcw->setRetransmitFlag(false);
	return BC_IS_UNLOCK_COMMAND;
}

ReturnValue_t Farm1StateOpen::handleBCSetVrCommand( ClcwIF* clcw, uint8_t vr ) {
	myVC->farmBCounter++;
	clcw->setRetransmitFlag(false);
	myVC->vR = vr;
	return BC_IS_SET_VR_COMMAND;
}
