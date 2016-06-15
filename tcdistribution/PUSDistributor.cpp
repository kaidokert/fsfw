/*
 * PUSDistributor.cpp
 *
 *  Created on: 18.06.2012
 *      Author: baetz
 */



#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tcdistribution/CCSDSDistributorIF.h>
#include <framework/tcdistribution/PUSDistributor.h>
#include <framework/tmtcpacket/pus/TcPacketStored.h>
#include <framework/tmtcservices/PusVerificationReport.h>

PUSDistributor::PUSDistributor(uint16_t set_apid ) : TcDistributor(objects::PUS_PACKET_DISTRIBUTOR), checker(set_apid), verify_channel(),
	current_packet(), tc_status(RETURN_FAILED) {

}

PUSDistributor::~PUSDistributor() {

}

iterator_t PUSDistributor::selectDestination() {
//	debug << "PUSDistributor::handlePacket received: " << this->current_packet_id.store_index << ", " << this->current_packet_id.packet_index << std::endl;
	iterator_t queueMapIt = this->queueMap.end();
	this->current_packet.setStoreAddress( this->currentMessage.getStorageId() );
	if ( current_packet.getWholeData() != NULL ) {
		tc_status = checker.checkPacket( &current_packet );
//		info << "PUSDistributor::handlePacket: packetCheck returned with " << (int)tc_status << std::endl;
		uint32_t queue_id = current_packet.getService();
		queueMapIt = this->queueMap.find( queue_id );
	} else {
		tc_status = PACKET_LOST;
	}
	if ( queueMapIt == this->queueMap.end() ) {
		tc_status = DESTINATION_NOT_FOUND;
	}

	if ( tc_status != RETURN_OK ) {
		debug << "PUSDistributor::handlePacket: error with " << (int)tc_status << std::endl;
		return this->queueMap.end();
	} else {
		return queueMapIt;
	}

}


//uint16_t PUSDistributor::createDestination( uint8_t service_id, uint8_t subservice_id ) {
//	return ( service_id << 8 ) + subservice_id;
//}

ReturnValue_t PUSDistributor::registerService(AcceptsTelecommandsIF* service) {
	ReturnValue_t returnValue = RETURN_OK;
	bool errorCode = true;
	uint16_t serviceId = service->getIdentifier();
	MessageQueueId_t queue = service->getRequestQueue();
	errorCode = this->queueMap.insert( std::pair<uint32_t, MessageQueueId_t>( serviceId,  queue) ).second;
	if(  errorCode == false ) {
		returnValue = OSAL::RESOURCE_IN_USE;
	}
	return returnValue;
}

MessageQueueId_t PUSDistributor::getRequestQueue() {
	return this->tcQueue.getId();
}

ReturnValue_t PUSDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
	if ( queueStatus != RETURN_OK ) {
		tc_status = queueStatus;
	}
	if ( tc_status != RETURN_OK ) {
		this->verify_channel.sendFailureReport(  TC_VERIFY::ACCEPTANCE_FAILURE, &current_packet, tc_status );
		//A failed packet is deleted immediately after reporting, otherwise it will block memory.
		current_packet.deletePacket();
		return RETURN_FAILED;
	} else {
		this->verify_channel.sendSuccessReport(  TC_VERIFY::ACCEPTANCE_SUCCESS, &current_packet );
		return RETURN_OK;
	}
}

uint16_t PUSDistributor::getIdentifier() {
	return checker.getApid();
}

ReturnValue_t PUSDistributor::initialize() {
	CCSDSDistributorIF* ccsdsDistributor = objectManager->get<CCSDSDistributorIF>(objects::CCSDS_PACKET_DISTRIBUTOR);
	if (ccsdsDistributor == NULL) {
		return RETURN_FAILED;
	} else {
		return ccsdsDistributor->registerApplication(this);
	}
}
