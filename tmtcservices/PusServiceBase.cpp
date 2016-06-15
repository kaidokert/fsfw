/*
 * PusServiceBase.cpp
 *
 *  Created on: May 9, 2012
 *      Author: baetz
 */
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tcdistribution/PUSDistributorIF.h>
#include <framework/tmtcservices/AcceptsTelemetryIF.h>
#include <framework/tmtcservices/PusServiceBase.h>
#include <framework/tmtcservices/PusVerificationReport.h>
#include <framework/tmtcservices/TmTcMessage.h>

PusServiceBase::PusServiceBase(object_id_t setObjectId, uint16_t set_apid,
		uint8_t set_service_id) :
		SystemObject(setObjectId), apid(set_apid), serviceId(set_service_id), errorParameter1(
				0), errorParameter2(0), requestQueue(PUS_SERVICE_MAX_RECEPTION) {
}

PusServiceBase::~PusServiceBase() {

}

ReturnValue_t PusServiceBase::performOperation() {
	TmTcMessage message;
	for (uint8_t count = 0; count < PUS_SERVICE_MAX_RECEPTION; count++) {
		ReturnValue_t status = this->requestQueue.receiveMessage(&message);
		//	debug << "PusServiceBase::performOperation: Receiving from MQ ID: " << std::hex << this->requestQueue.getId() << std::dec << " returned: " << status << std::endl;
		if (status == RETURN_OK) {
			this->currentPacket.setStoreAddress(message.getStorageId());
//			info << "Service " << (uint16_t) this->serviceId << ": new packet!"
//					<< std::endl;

			ReturnValue_t return_code = this->handleRequest();
			//		debug << "Service " << (uint16_t)this->serviceId << ": handleRequest returned: " << (int)return_code << std::endl;
			if (return_code == RETURN_OK) {
				this->verifyReporter.sendSuccessReport(
						TC_VERIFY::COMPLETION_SUCCESS, &this->currentPacket);
			} else {
				this->verifyReporter.sendFailureReport(
						TC_VERIFY::COMPLETION_FAILURE, &this->currentPacket,
						return_code, 0, errorParameter1, errorParameter2);
			}
			this->currentPacket.deletePacket();
			errorParameter1 = 0;
			errorParameter2 = 0;
		} else if (status == OSAL::QUEUE_EMPTY) {
			status = RETURN_OK;
			//		debug << "PusService " << (uint16_t)this->serviceId << ": no new packet." << std::endl;
			break;
		} else {

			error << "PusServiceBase::performOperation: Service "
					<< (uint16_t) this->serviceId
					<< ": Error receiving packet. Code: " << std::hex << status
					<< std::dec << std::endl;
		}
	}
	ReturnValue_t return_code = this->performService();
	if (return_code == RETURN_OK) {
		return RETURN_OK;
	} else {

		error << "PusService " << (uint16_t) this->serviceId
				<< ": performService returned with " << (int16_t) return_code
				<< std::endl;
		return RETURN_FAILED;
	}
}

uint16_t PusServiceBase::getIdentifier() {
	return this->serviceId;
}

MessageQueueId_t PusServiceBase::getRequestQueue() {
	return this->requestQueue.getId();
}

ReturnValue_t PusServiceBase::initialize() {
	ReturnValue_t result = SystemObject::initialize();
	if (result != RETURN_OK) {
		return result;
	}
	AcceptsTelemetryIF* dest_service = objectManager->get<AcceptsTelemetryIF>(
			objects::PUS_PACKET_FORWARDING);
	PUSDistributorIF* distributor = objectManager->get<PUSDistributorIF>(
			objects::PUS_PACKET_DISTRIBUTOR);
	if ((dest_service != NULL) && (distributor != NULL)) {
		this->requestQueue.setDefaultDestination(
				dest_service->getReportReceptionQueue());
		distributor->registerService(this);
		return RETURN_OK;
	} else {
		error << "PusServiceBase::PusServiceBase: Service "
				<< (uint32_t) this->serviceId << ": Configuration error."
				<< std::endl;
		return RETURN_FAILED;
	}
}
