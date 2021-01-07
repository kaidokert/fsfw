#include "CCSDSDistributorIF.h"
#include "PUSDistributor.h"

#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../tmtcpacket/pus/TcPacketStored.h"
#include "../tmtcservices/PusVerificationReport.h"

PUSDistributor::PUSDistributor(uint16_t setApid, object_id_t setObjectId,
		object_id_t setPacketSource) :
		TcDistributor(setObjectId), checker(setApid), verifyChannel(),
		tcStatus(RETURN_FAILED), packetSource(setPacketSource) {}

PUSDistributor::~PUSDistributor() {}

PUSDistributor::TcMqMapIter PUSDistributor::selectDestination() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    // sif:: debug << "PUSDistributor::handlePacket received: "
    //          << this->current_packet_id.store_index << ", "
    //          << this->current_packet_id.packet_index << std::endl;
#endif
    TcMqMapIter queueMapIt = this->queueMap.end();
    if(this->currentPacket == nullptr) {
        return queueMapIt;
    }
    this->currentPacket->setStoreAddress(this->currentMessage.getStorageId());
    if (currentPacket->getWholeData() != nullptr) {
        tcStatus = checker.checkPacket(currentPacket);
#ifdef DEBUG
        if(tcStatus != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::debug << "PUSDistributor::handlePacket: Packet format "
                    << "invalid, code "<< static_cast<int>(tcStatus)
                    << std::endl;
#endif
        }
#endif
        uint32_t queue_id = currentPacket->getService();
        queueMapIt = this->queueMap.find(queue_id);
    }
    else {
        tcStatus = PACKET_LOST;
    }

    if (queueMapIt == this->queueMap.end()) {
        tcStatus = DESTINATION_NOT_FOUND;
#ifdef DEBUG
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "PUSDistributor::handlePacket: Destination not found, "
                << "code "<< static_cast<int>(tcStatus) << std::endl;
#endif
#endif
    }

    if (tcStatus != RETURN_OK) {
        return this->queueMap.end();
    }
    else {
        return queueMapIt;
    }

}


ReturnValue_t PUSDistributor::registerService(AcceptsTelecommandsIF* service) {
	uint16_t serviceId = service->getIdentifier();
#if FSFW_CPP_OSTREAM_ENABLED == 1
	// sif::info << "Service ID: " << (int)serviceId << std::endl;
#endif
	MessageQueueId_t queue = service->getRequestQueue();
	auto returnPair = queueMap.emplace(serviceId, queue);
	if (not returnPair.second) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "PUSDistributor::registerService: Service ID already"
				" exists in map." << std::endl;
#endif
		return SERVICE_ID_ALREADY_EXISTS;
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
		this->verifyChannel.sendFailureReport(tc_verification::ACCEPTANCE_FAILURE,
				currentPacket, tcStatus);
		// A failed packet is deleted immediately after reporting,
		// otherwise it will block memory.
		currentPacket->deletePacket();
		return RETURN_FAILED;
	} else {
		this->verifyChannel.sendSuccessReport(tc_verification::ACCEPTANCE_SUCCESS,
				currentPacket);
		return RETURN_OK;
	}
}

uint16_t PUSDistributor::getIdentifier() {
	return checker.getApid();
}

ReturnValue_t PUSDistributor::initialize() {
    currentPacket = new TcPacketStored();
    if(currentPacket == nullptr) {
        // Should not happen, memory allocation failed!
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }

	CCSDSDistributorIF* ccsdsDistributor =
			objectManager->get<CCSDSDistributorIF>(packetSource);
	if (ccsdsDistributor == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "PUSDistributor::initialize: Packet source invalid."
		        << " Make sure it exists and implements CCSDSDistributorIF!"
		        << std::endl;
#endif
	    return RETURN_FAILED;
	}
	return ccsdsDistributor->registerApplication(this);
}
