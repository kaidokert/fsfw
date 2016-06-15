/**
 * @file	PollingSlot.cpp
 * @brief	This file defines the PollingSlot class.
 * @date	19.12.2012
 * @author	baetz
 */

#include <framework/devicehandlers/PollingSlot.h>
#include <framework/objectmanager/SystemObjectIF.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

PollingSlot::PollingSlot( object_id_t handlerId, Interval_t setTime, int8_t setSequenceId ) {
	//Set all attributes directly on object creation.
	this->handler = objectManager->get<PollingSequenceExecutableIF>( handlerId );
	this->pollingTime = setTime;
	this->opcode = setSequenceId;
}

PollingSlot::~PollingSlot() {
}

void PollingSlot::print() const {
	SystemObjectIF * systemObject = dynamic_cast<SystemObjectIF *>(handler);
	object_id_t id = (systemObject == NULL) ? 0xffffffff : systemObject->getObjectId();
	debug << "HandlerID: " << std::hex << id << std::dec << "; Polling Time: "
			<< this->pollingTime << "; Opcode: " << (uint16_t) this->opcode
			<< std::endl;
}

