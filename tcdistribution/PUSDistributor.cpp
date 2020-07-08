#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tcdistribution/CCSDSDistributorIF.h>
#include <framework/tcdistribution/PUSDistributor.h>
#include <framework/tmtcpacket/pus/TcPacketStored.h>
#include <framework/tmtcservices/PusVerificationReport.h>

PUSDistributor::PUSDistributor(uint16_t setApid, object_id_t setObjectId,
		object_id_t setPacketSource) :
		TcDistributor(setObjectId), checker(setApid), verifyChannel(),
		currentPacket(), tcStatus(RETURN_FAILED),
		packetSource(setPacketSource) {}

PUSDistributor::~PUSDistributor() {}

TcDistributor::TcMessageQueueMapIter PUSDistributor::selectDestination() {
//	debug << "PUSDistributor::handlePacket received: " << this->current_packet_id.store_index << ", " << this->current_packet_id.packet_index << std::endl;
	TcMessageQueueMapIter queueMapIt = this->queueMap.end();
	this->currentPacket.setStoreAddress(this->currentMessage.getStorageId());
	if (currentPacket.getWholeData() != NULL) {
		tcStatus = checker.checkPacket(&currentPacket);
//		info << "PUSDistributor::handlePacket: packetCheck returned with " << (int)tc_status << std::endl;
		uint32_t queue_id = currentPacket.getService();
		queueMapIt = this->queueMap.find(queue_id);
	} else {
		tcStatus = PACKET_LOST;
	}
	if (queueMapIt == this->queueMap.end()) {
		tcStatus = DESTINATION_NOT_FOUND;
	}

	if (tcStatus != RETURN_OK) {
		sif::debug << "PUSDistributor::handlePacket: error with " << (int) tcStatus
				<< ", 0x"<< std::hex << (int) tcStatus << std::endl;
		return this->queueMap.end();
	} else {
		return queueMapIt;
	}

}

//uint16_t PUSDistributor::createDestination( uint8_t service_id, uint8_t subservice_id ) {
//	return ( service_id << 8 ) + subservice_id;
//}

ReturnValue_t PUSDistributor::registerService(AcceptsTelecommandsIF* service) {
	uint16_t serviceId = service->getIdentifier();
	//info << "Service ID: " << (int)serviceId << std::endl;
	MessageQueueId_t queue = service->getRequestQueue();
	auto returnPair = queueMap.emplace(serviceId, queue);
	if (not returnPair.second) {
		//TODO Return Code
		sif::error << "PUSDistributor::registerService: Service ID already"
				"exists in map." << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t PUSDistributor::getRequestQueue() {
	return tcQueue->getId();
}

ReturnValue_t PUSDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
	if (queueStatus != RETURN_OK) {
		tcStatus = queueStatus;
	}
	if (tcStatus != RETURN_OK) {
		this->verifyChannel.sendFailureReport(TC_VERIFY::ACCEPTANCE_FAILURE,
				&currentPacket, tcStatus);
		//A failed packet is deleted immediately after reporting, otherwise it will block memory.
		currentPacket.deletePacket();
		return RETURN_FAILED;
	} else {
		this->verifyChannel.sendSuccessReport(TC_VERIFY::ACCEPTANCE_SUCCESS,
				&currentPacket);
		return RETURN_OK;
	}
}

uint16_t PUSDistributor::getIdentifier() {
	return checker.getApid();
}

ReturnValue_t PUSDistributor::initialize() {
	CCSDSDistributorIF* ccsdsDistributor =
			objectManager->get<CCSDSDistributorIF>(packetSource);
	if (ccsdsDistributor == NULL) {
		return RETURN_FAILED;
	} else {
		return ccsdsDistributor->registerApplication(this);
	}
}
