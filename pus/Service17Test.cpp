#include "Service17Test.h"

#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../objectmanager/SystemObject.h"
#include "../tmtcpacket/pus/TmPacketStored.h"


Service17Test::Service17Test(object_id_t objectId,
        uint16_t apid, uint8_t serviceId):
    PusServiceBase(objectId, apid, serviceId),
    packetSubCounter(0) {
}

Service17Test::~Service17Test() {
}

ReturnValue_t Service17Test::handleRequest(uint8_t subservice) {
	switch(subservice){
	case Subservice::CONNECTION_TEST: {
		TmPacketStored connectionPacket(apid, serviceId,
		        Subservice::CONNECTION_TEST_REPORT, packetSubCounter++);
		connectionPacket.sendPacket(requestQueue->getDefaultDestination(),
				requestQueue->getId());
		return HasReturnvaluesIF::RETURN_OK;
	}
    case Subservice::EVENT_TRIGGER_TEST: {
        TmPacketStored connectionPacket(apid, serviceId,
                Subservice::CONNECTION_TEST_REPORT, packetSubCounter++);
        connectionPacket.sendPacket(requestQueue->getDefaultDestination(),
                requestQueue->getId());
        triggerEvent(TEST, 1234, 5678);
        return RETURN_OK;
    }
	default:
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}

ReturnValue_t Service17Test::performService() {
	return HasReturnvaluesIF::RETURN_OK;
}
