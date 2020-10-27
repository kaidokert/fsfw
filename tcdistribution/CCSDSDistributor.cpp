#include "CCSDSDistributor.h"

#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../tmtcpacket/SpacePacketBase.h"

CCSDSDistributor::CCSDSDistributor(uint16_t setDefaultApid,
		object_id_t setObjectId):
		TcDistributor(setObjectId), defaultApid( setDefaultApid ) {
}

CCSDSDistributor::~CCSDSDistributor() {}

TcDistributor::TcMqMapIter CCSDSDistributor::selectDestination() {
//	sif::debug << "CCSDSDistributor::selectDestination received: " <<
//			this->currentMessage.getStorageId().pool_index << ", " <<
//			this->currentMessage.getStorageId().packet_index << std::endl;
	const uint8_t* packet = nullptr;
	size_t size = 0;
	ReturnValue_t result = this->tcStore->getData(currentMessage.getStorageId(),
			&packet, &size );
	if(result != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "CCSDSDistributor::selectDestination: Getting data from"
				" store failed!" << std::endl;
	}
	SpacePacketBase currentPacket(packet);

//	sif:: info << "CCSDSDistributor::selectDestination has packet with APID "
//			<< std::hex << currentPacket.getAPID() << std::dec << std::endl;
	TcMqMapIter position = this->queueMap.find(currentPacket.getAPID());
	if ( position != this->queueMap.end() ) {
		return position;
	} else {
		//The APID was not found. Forward packet to main SW-APID anyway to
		// create acceptance failure report.
		return this->queueMap.find( this->defaultApid );
	}
}

MessageQueueId_t CCSDSDistributor::getRequestQueue() {
	return tcQueue->getId();
}

ReturnValue_t CCSDSDistributor::registerApplication(
		AcceptsTelecommandsIF* application) {
	ReturnValue_t returnValue = RETURN_OK;
	auto insertPair = this->queueMap.emplace(application->getIdentifier(),
	        application->getRequestQueue());
	if(not insertPair.second) {
		returnValue = RETURN_FAILED;
	}
	return returnValue;
}

ReturnValue_t CCSDSDistributor::registerApplication(uint16_t apid,
		MessageQueueId_t id) {
	ReturnValue_t returnValue = RETURN_OK;
	auto insertPair = this->queueMap.emplace(apid, id);
	if(not insertPair.second) {
		returnValue = RETURN_FAILED;
	}
	return returnValue;

}

uint16_t CCSDSDistributor::getIdentifier() {
	return 0;
}

ReturnValue_t CCSDSDistributor::initialize() {
	ReturnValue_t status = this->TcDistributor::initialize();
	this->tcStore = objectManager->get<StorageManagerIF>( objects::TC_STORE );
	if (this->tcStore == nullptr) {
	    sif::error << "CCSDSDistributor::initialize: Could not initialize"
	            " TC store!" << std::endl;
	    status = RETURN_FAILED;
	}
	return status;
}

ReturnValue_t CCSDSDistributor::callbackAfterSending(
		ReturnValue_t queueStatus) {
	if (queueStatus != RETURN_OK) {
		tcStore->deleteData(currentMessage.getStorageId());
	}
	return RETURN_OK;
}
