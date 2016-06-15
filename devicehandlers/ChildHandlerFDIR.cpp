/*
 * ChildHandlerFDIR.cpp
 *
 *  Created on: 08.02.2016
 *      Author: baetz
 */

#include <framework/devicehandlers/ChildHandlerFDIR.h>

ChildHandlerFDIR::ChildHandlerFDIR(object_id_t owner, object_id_t faultTreeParent, uint32_t recoveryCount) :
		DeviceHandlerFDIR(owner, faultTreeParent) {
	recoveryCounter.setFailureThreshold(recoveryCount);
}

ChildHandlerFDIR::~ChildHandlerFDIR() {
}

