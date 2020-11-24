#include "TcPacketCheck.h"

#include "../globalfunctions/CRC.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../tmtcservices/VerificationCodes.h"

TcPacketCheck::TcPacketCheck( uint16_t setApid ) : apid(setApid) {
}

ReturnValue_t TcPacketCheck::checkPacket( TcPacketStored* currentPacket ) {
	uint16_t calculated_crc = CRC::crc16ccitt( currentPacket->getWholeData(),
	        currentPacket->getFullSize() );
	if ( calculated_crc != 0 ) {
		return INCORRECT_CHECKSUM;
	}
	bool condition = (not currentPacket->hasSecondaryHeader()) or
			(currentPacket->getPacketVersionNumber() != CCSDS_VERSION_NUMBER) or
			(not currentPacket->isTelecommand());
	if ( condition ) {
		return INCORRECT_PRIMARY_HEADER;
	}
	if ( currentPacket->getAPID() != this->apid )
		return ILLEGAL_APID;

	if ( not currentPacket->isSizeCorrect() ) {
		return INCOMPLETE_PACKET;
	}
	condition = (currentPacket->getSecondaryHeaderFlag() != CCSDS_SECONDARY_HEADER_FLAG) ||
			(currentPacket->getPusVersionNumber() != PUS_VERSION_NUMBER);
	if ( condition ) {
		return INCORRECT_SECONDARY_HEADER;
	}
	return RETURN_OK;
}

uint16_t TcPacketCheck::getApid() const {
	return apid;
}
