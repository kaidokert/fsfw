/*
 * CCSDSDistributor.cpp
 *
 *  Created on: 18.06.2012
 *      Author: baetz
 */

#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tcdistribution/CCSDSDistributor.h>
#include <framework/tmtcpacket/SpacePacketBase.h>

CCSDSDistributor::CCSDSDistributor( uint16_t set_default_apid ) :
		TcDistributor( objects::CCSDS_PACKET_DISTRIBUTOR ), default_apid( set_default_apid ), tcStore(NULL) {
}

CCSDSDistributor::~CCSDSDistributor() {

}

iterator_t CCSDSDistributor::selectDestination() {
//	debug << "CCSDSDistributor::selectDestination received: " << this->currentMessage.getStorageId().pool_index << ", " << this->currentMessage.getStorageId().packet_index << std::endl;
	const uint8_t* p_packet = NULL;
	uint32_t size = 0;
	this->tcStore->getData( this->currentMessage.getStorageId(), &p_packet, &size );
	SpacePacketBase current_packet( p_packet );
//	info << "CCSDSDistributor::selectDestination has packet with APID " << std::hex << current_packet.getAPID() << std::dec << std::endl;
	iterator_t position = this->queueMap.find( current_packet.getAPID() );
	if ( position != this->queueMap.end() ) {
		return position;
	} else {
		//The APID was not found. Forward packet to main SW-APID anyway to create acceptance failure report.
		return this->queueMap.find( this->default_apid );
	}

}

MessageQueueId_t CCSDSDistributor::getRequestQueue() {
	return this->tcQueue.getId();
}

ReturnValue_t CCSDSDistributor::registerApplication(
		AcceptsTelecommandsIF* application) {
	ReturnValue_t returnValue = RETURN_OK;
	bool errorCode = true;
	errorCode = this->queueMap.insert( std::pair<uint32_t, MessageQueueId_t>( application->getIdentifier(), application->getRequestQueue() ) ).second;
	if(  errorCode == false ) {
		returnValue = RETURN_FAILED;
	}
	return returnValue;
}

ReturnValue_t CCSDSDistributor::registerApplication(uint16_t apid,
		MessageQueueId_t id) {
	ReturnValue_t returnValue = RETURN_OK;
	bool errorCode = true;
	errorCode = this->queueMap.insert( std::pair<uint32_t, MessageQueueId_t>( apid, id ) ).second;
	if(  errorCode == false ) {
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
	if (this->tcStore == NULL) status = RETURN_FAILED;
	return status;
}

ReturnValue_t CCSDSDistributor::callbackAfterSending(
		ReturnValue_t queueStatus) {
	if (queueStatus != RETURN_OK) {
		tcStore->deleteData(currentMessage.getStorageId());
	}
	return RETURN_OK;
}
