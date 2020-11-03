#include "Service9TimeManagement.h"
#include "servicepackets/Service9Packets.h"

#include "../timemanager/CCSDSTime.h"
#include "../events/EventManagerIF.h"
#include "../serviceinterface/ServiceInterfaceStream.h"


Service9TimeManagement::Service9TimeManagement(object_id_t objectId,
		uint16_t apid, uint8_t serviceId) :
	PusServiceBase(objectId, apid , serviceId) {
}

Service9TimeManagement::~Service9TimeManagement() {}

ReturnValue_t Service9TimeManagement::performService() {
	return RETURN_OK;
}

ReturnValue_t Service9TimeManagement::handleRequest(uint8_t subservice) {
	switch(subservice){
		case SUBSERVICE::SET_TIME:{
			return setTime();
		}
		default:
			return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
		}
}

ReturnValue_t Service9TimeManagement::setTime() {
	Clock::TimeOfDay_t timeToSet;
	TimePacket timePacket(currentPacket.getApplicationData(),
			currentPacket.getApplicationDataSize());
	ReturnValue_t result = CCSDSTime::convertFromCcsds(&timeToSet,
			timePacket.getTime(), timePacket.getTimeSize());
	if(result != RETURN_OK) {
		triggerEvent(CLOCK_SET_FAILURE, result, 0);
		return result;
	}

	uint32_t formerUptime;
	Clock::getUptime(&formerUptime);
	result = Clock::setClock(&timeToSet);

	if(result == RETURN_OK) {
		uint32_t newUptime;
		Clock::getUptime(&newUptime);
		triggerEvent(CLOCK_SET,newUptime,formerUptime);
		return RETURN_OK;
	}
	else {
		triggerEvent(CLOCK_SET_FAILURE, result, 0);
		return RETURN_FAILED;
	}
}



