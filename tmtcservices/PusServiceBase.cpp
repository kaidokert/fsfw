#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../tcdistribution/PUSDistributorIF.h"
#include "AcceptsTelemetryIF.h"
#include "PusServiceBase.h"
#include "PusVerificationReport.h"
#include "TmTcMessage.h"
#include "../ipc/QueueFactory.h"

object_id_t PusServiceBase::packetSource = 0;
object_id_t PusServiceBase::packetDestination = 0;

PusServiceBase::PusServiceBase(object_id_t setObjectId, uint16_t setApid,
		uint8_t setServiceId) :
		SystemObject(setObjectId), apid(setApid), serviceId(setServiceId) {
	requestQueue = QueueFactory::instance()->
			createMessageQueue(PUS_SERVICE_MAX_RECEPTION);
}

PusServiceBase::~PusServiceBase() {
	QueueFactory::instance()->deleteMessageQueue(requestQueue);
}

ReturnValue_t PusServiceBase::performOperation(uint8_t opCode) {
	handleRequestQueue();
	ReturnValue_t result = this->performService();
	if (result != RETURN_OK) {
		sif::error << "PusService " << (uint16_t) this->serviceId
				<< ": performService returned with " << (int16_t) result
				<< std::endl;
		return RETURN_FAILED;
	}
	return RETURN_OK;
}

void PusServiceBase::setTaskIF(PeriodicTaskIF* taskHandle) {
    this->taskHandle = taskHandle;
}

void PusServiceBase::handleRequestQueue() {
	TmTcMessage message;
	ReturnValue_t result = RETURN_FAILED;
	for (uint8_t count = 0; count < PUS_SERVICE_MAX_RECEPTION; count++) {
		ReturnValue_t status = this->requestQueue->receiveMessage(&message);
		//	debug << "PusServiceBase::performOperation: Receiving from MQ ID: "
		//      << std::hex << this->requestQueue.getId()
		//      << std::dec << " returned: " << status << std::endl;
		if (status == RETURN_OK) {
			this->currentPacket.setStoreAddress(message.getStorageId());
			//info << "Service " << (uint16_t) this->serviceId <<
			//     ": new packet!" << std::endl;

			result = this->handleRequest(currentPacket.getSubService());

			// debug << "Service " << (uint16_t)this->serviceId <<
			//    ": handleRequest returned: " << (int)return_code << std::endl;
			if (result == RETURN_OK) {
				this->verifyReporter.sendSuccessReport(
						TC_VERIFY::COMPLETION_SUCCESS, &this->currentPacket);
			}
			else {
				this->verifyReporter.sendFailureReport(
						TC_VERIFY::COMPLETION_FAILURE, &this->currentPacket,
						result, 0, errorParameter1, errorParameter2);
			}
			this->currentPacket.deletePacket();
			errorParameter1 = 0;
			errorParameter2 = 0;
		}
		else if (status == MessageQueueIF::EMPTY) {
			status = RETURN_OK;
			// debug << "PusService " << (uint16_t)this->serviceId <<
			//      ": no new packet." << std::endl;
			break;
		}
		else {
			sif::error << "PusServiceBase::performOperation: Service "
					<< (uint16_t) this->serviceId
					<< ": Error receiving packet. Code: " << std::hex << status
					<< std::dec << std::endl;
		}
	}
}

uint16_t PusServiceBase::getIdentifier() {
	return this->serviceId;
}

MessageQueueId_t PusServiceBase::getRequestQueue() {
	return this->requestQueue->getId();
}

ReturnValue_t PusServiceBase::initialize() {
	ReturnValue_t result = SystemObject::initialize();
	if (result != RETURN_OK) {
		return result;
	}
	AcceptsTelemetryIF* destService = objectManager->get<AcceptsTelemetryIF>(
			packetDestination);
	PUSDistributorIF* distributor = objectManager->get<PUSDistributorIF>(
			packetSource);
	if ((destService != nullptr) && (distributor != nullptr)) {
		this->requestQueue->setDefaultDestination(
				destService->getReportReceptionQueue());
		distributor->registerService(this);
		return RETURN_OK;
	}
	else {
		sif::error << "PusServiceBase::PusServiceBase: Service "
				<< (uint32_t) this->serviceId << ": Configuration error."
				<< " Make sure packetSource and packetDestination are defined "
			       "correctly" << std::endl;
		return RETURN_FAILED;
	}
}

ReturnValue_t PusServiceBase::initializeAfterTaskCreation() {
    // If task parameters, for example task frequency are required, this
    // function should be overriden and the system object task IF can
    // be used to get those parameters.
    return HasReturnvaluesIF::RETURN_OK;
}
