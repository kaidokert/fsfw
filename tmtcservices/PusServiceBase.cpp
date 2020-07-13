#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tcdistribution/PUSDistributorIF.h>
#include <framework/tmtcservices/AcceptsTelemetryIF.h>
#include <framework/tmtcservices/PusServiceBase.h>
#include <framework/tmtcservices/PusVerificationReport.h>
#include <framework/tmtcservices/TmTcMessage.h>
#include <framework/ipc/QueueFactory.h>

object_id_t PusServiceBase::packetSource = 0;
object_id_t PusServiceBase::packetDestination = 0;

PusServiceBase::PusServiceBase(object_id_t setObjectId, uint16_t setApid, uint8_t setServiceId) :
		SystemObject(setObjectId), apid(setApid), serviceId(setServiceId), errorParameter1(
				0), errorParameter2(0), requestQueue(NULL) {
	requestQueue = QueueFactory::instance()->createMessageQueue(PUS_SERVICE_MAX_RECEPTION);
}

PusServiceBase::~PusServiceBase() {
	QueueFactory::instance()->deleteMessageQueue(requestQueue);
}

ReturnValue_t PusServiceBase::performOperation(uint8_t opCode) {
	TmTcMessage message;
	for (uint8_t count = 0; count < PUS_SERVICE_MAX_RECEPTION; count++) {
		ReturnValue_t status = this->requestQueue->receiveMessage(&message);
		//	debug << "PusServiceBase::performOperation: Receiving from MQ ID: " << std::hex << this->requestQueue.getId()
		// << std::dec << " returned: " << status << std::endl;
		if (status == RETURN_OK) {
			this->currentPacket.setStoreAddress(message.getStorageId());
			//	info << "Service " << (uint16_t) this->serviceId << ": new packet!" << std::endl;

			ReturnValue_t return_code = this->handleRequest();
			// debug << "Service " << (uint16_t)this->serviceId << ": handleRequest returned: " << (int)return_code << std::endl;
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
		} else if (status == MessageQueueIF::EMPTY) {
			status = RETURN_OK;
			// debug << "PusService " << (uint16_t)this->serviceId << ": no new packet." << std::endl;
			break;
		} else {

			sif::error << "PusServiceBase::performOperation: Service "
					<< (uint16_t) this->serviceId
					<< ": Error receiving packet. Code: " << std::hex << status
					<< std::dec << std::endl;
		}
	}
	ReturnValue_t return_code = this->performService();
	if (return_code == RETURN_OK) {
		return RETURN_OK;
	} else {

		sif::error << "PusService " << (uint16_t) this->serviceId
				<< ": performService returned with " << (int16_t) return_code
				<< std::endl;
		return RETURN_FAILED;
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
	AcceptsTelemetryIF* dest_service = objectManager->get<AcceptsTelemetryIF>(
			packetDestination);
	PUSDistributorIF* distributor = objectManager->get<PUSDistributorIF>(
			packetSource);
	if ((dest_service != NULL) && (distributor != NULL)) {
		this->requestQueue->setDefaultDestination(
				dest_service->getReportReceptionQueue());
		distributor->registerService(this);
		return RETURN_OK;
	} else {
		sif::error << "PusServiceBase::PusServiceBase: Service "
				<< (uint32_t) this->serviceId << ": Configuration error."
				<< " Make sure packetSource and packetDestination are defined correctly" << std::endl;
		return RETURN_FAILED;
	}
}
