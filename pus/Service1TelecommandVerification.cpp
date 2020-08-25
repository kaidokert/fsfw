#include "Service1TelecommandVerification.h"
#include "servicepackets/Service1Packets.h"

#include "../ipc/QueueFactory.h"
#include "../tmtcservices/PusVerificationReport.h"
#include "../tmtcpacket/pus/TmPacketStored.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../tmtcservices/AcceptsTelemetryIF.h"
#include "../serviceinterface/ServiceInterfaceStream.h"


Service1TelecommandVerification::Service1TelecommandVerification(
        object_id_t objectId, uint16_t apid, uint8_t serviceId,
        object_id_t targetDestination):
        SystemObject(objectId), apid(apid), serviceId(serviceId),
        targetDestination(targetDestination) {
	tmQueue = QueueFactory::instance()->createMessageQueue();
}

Service1TelecommandVerification::~Service1TelecommandVerification() {}

MessageQueueId_t Service1TelecommandVerification::getVerificationQueue(){
	return tmQueue->getId();
}


ReturnValue_t Service1TelecommandVerification::performOperation(
		uint8_t operationCode){
	PusVerificationMessage message;
	ReturnValue_t status = tmQueue->receiveMessage(&message);
	while(status == HasReturnvaluesIF::RETURN_OK) {
		status = sendVerificationReport(&message);
		if(status != HasReturnvaluesIF::RETURN_OK) {
			return status;
		}
		status = tmQueue->receiveMessage(&message);
	}
	if (status == MessageQueueIF::EMPTY) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return status;
	}
}


ReturnValue_t Service1TelecommandVerification::sendVerificationReport(
		PusVerificationMessage* message) {
	ReturnValue_t result;
	if(message->getReportId() % 2 == 0) {
		result = generateFailureReport(message);
	} else {
		result = generateSuccessReport(message);
	}
	if(result != HasReturnvaluesIF::RETURN_OK){
		sif::error << "Service1TelecommandVerification::initialize: "
		        "Sending verification packet failed !" << std::endl;
	}
	return result;
}

ReturnValue_t Service1TelecommandVerification::generateFailureReport(
        PusVerificationMessage *message) {
	FailureReport report(
			message->getReportId(), message->getTcPacketId(),
			message->getTcSequenceControl(), message->getStep(),
			message->getErrorCode(), message->getParameter1(),
			message->getParameter2());
	TmPacketStored tmPacket(apid, serviceId, message->getReportId(),
	        packetSubCounter++, &report);
	ReturnValue_t result = tmPacket.sendPacket(tmQueue->getDefaultDestination(),
			tmQueue->getId());
	return result;
}

ReturnValue_t Service1TelecommandVerification::generateSuccessReport(
        PusVerificationMessage *message) {
	SuccessReport report(message->getReportId(),message->getTcPacketId(),
			message->getTcSequenceControl(),message->getStep());
	TmPacketStored tmPacket(apid, serviceId, message->getReportId(),
	        packetSubCounter++, &report);
	ReturnValue_t result = tmPacket.sendPacket(tmQueue->getDefaultDestination(),
			tmQueue->getId());
	return result;
}


ReturnValue_t Service1TelecommandVerification::initialize() {
	// Get target object for TC verification messages
	AcceptsTelemetryIF* funnel = objectManager->
			get<AcceptsTelemetryIF>(targetDestination);
	if(funnel == nullptr){
	    sif::error << "Service1TelecommandVerification::initialize: Specified"
	            " TM funnel invalid. Make sure it is set up and implements"
	            " AcceptsTelemetryIF." << std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}
	tmQueue->setDefaultDestination(funnel->getReportReceptionQueue());
	return SystemObject::initialize();
}
