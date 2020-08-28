#include "../globalfunctions/CRC.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../tcdistribution/TcPacketCheck.h"
#include "../tmtcservices/VerificationCodes.h"

TcPacketCheck::TcPacketCheck( uint16_t set_apid ) : apid(set_apid) {
}

ReturnValue_t TcPacketCheck::checkPacket( TcPacketStored* current_packet ) {
	uint16_t calculated_crc = CRC::crc16ccitt( current_packet->getWholeData(), current_packet->getFullSize() );
	if ( calculated_crc != 0 ) {
		return INCORRECT_CHECKSUM;
	}
	bool condition = !(current_packet->hasSecondaryHeader()) ||
			current_packet->getPacketVersionNumber() != CCSDS_VERSION_NUMBER ||
			!(current_packet->isTelecommand());
	if ( condition ) {
		return INCORRECT_PRIMARY_HEADER;
	}
	if ( current_packet->getAPID() != this->apid )
		return ILLEGAL_APID;

	if ( !current_packet->isSizeCorrect() ) {
		return INCOMPLETE_PACKET;
	}
	condition = (current_packet->getSecondaryHeaderFlag() != CCSDS_SECONDARY_HEADER_FLAG) ||
			(current_packet->getPusVersionNumber() != PUS_VERSION_NUMBER);
	if ( condition ) {
		return INCORRECT_SECONDARY_HEADER;
	}
	return RETURN_OK;
}

uint16_t TcPacketCheck::getApid() const {
	return apid;
}
