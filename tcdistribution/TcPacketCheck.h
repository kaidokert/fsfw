#ifndef FSFW_TCDISTRIBUTION_TCPACKETCHECK_H_
#define FSFW_TCDISTRIBUTION_TCPACKETCHECK_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include "../tmtcpacket/pus/TcPacketStored.h"
#include "../tmtcservices/PusVerificationReport.h"

/**
 * This class performs a formal packet check for incoming PUS Telecommand Packets.
 * Currently, it only checks if the APID and CRC are correct.
 * @ingroup tc_distribution
 */
class TcPacketCheck : public HasReturnvaluesIF {
protected:
	/**
	 * Describes the version number a packet must have to pass.
	 */
	static constexpr uint8_t CCSDS_VERSION_NUMBER = 0;
	/**
	 * Describes the secondary header a packet must have to pass.
	 */
	static constexpr uint8_t CCSDS_SECONDARY_HEADER_FLAG = 0;
	/**
	 * Describes the TC Packet PUS Version Number a packet must have to pass.
	 */
	static constexpr uint8_t PUS_VERSION_NUMBER = 1;
	/**
	 * The packet id each correct packet should have.
	 * It is composed of the APID and some static fields.
	 */
	uint16_t apid;
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::TC_PACKET_CHECK;
	static const ReturnValue_t ILLEGAL_APID = MAKE_RETURN_CODE( 0 );
	static const ReturnValue_t INCOMPLETE_PACKET = MAKE_RETURN_CODE( 1 );
	static const ReturnValue_t INCORRECT_CHECKSUM = MAKE_RETURN_CODE( 2 );
	static const ReturnValue_t ILLEGAL_PACKET_TYPE = MAKE_RETURN_CODE( 3 );
	static const ReturnValue_t ILLEGAL_PACKET_SUBTYPE = MAKE_RETURN_CODE( 4 );
	static const ReturnValue_t INCORRECT_PRIMARY_HEADER = MAKE_RETURN_CODE( 5 );
	static const ReturnValue_t INCORRECT_SECONDARY_HEADER = MAKE_RETURN_CODE( 6 );
	/**
	 * The constructor only sets the APID attribute.
	 * @param set_apid The APID to set.
	 */
	TcPacketCheck( uint16_t setApid );
	/**
	 * This is the actual method to formally check a certain Telecommand Packet.
	 * The packet's Application Data can not be checked here.
	 * @param current_packet The packt to check
	 * @return	- @c RETURN_OK on success.
	 * 			- @c INCORRECT_CHECKSUM if checksum is invalid.
	 * 			- @c ILLEGAL_APID if APID does not match.
	 */
	ReturnValue_t checkPacket( TcPacketStored* currentPacket );

	uint16_t getApid() const;
};


#endif /* FSFW_TCDISTRIBUTION_TCPACKETCHECK_H_ */
