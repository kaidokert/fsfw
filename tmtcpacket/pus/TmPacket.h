#ifndef TMPACKET_H_
#define TMPACKET_H_

#include <framework/tmtcpacket/pus/TmPacketBase.h>

/**
 * The TmPacket class is a representation of a ECSS PUS Telemetry packet.

 * The template parameter is used to instantiate the class with different
 * maximum Application Data sizes (to avoid wasting stack size). Almost every
 * on-board service is a source of telemetry packets. These packets are created
 * with the TmPacket class, which is instantiated similar to the TcPacket
 * class.
 * A pointer to \c local_data is passed to the \c TmPacketBase parent class,
 * so the parent's methods are reachable.
 * @t_param	byte_size	The maximum size of the complete packet (including CRC
 * 		and headers)
 * @ingroup tmtcpackets
 */
template<uint32_t byte_size>
class TmPacket : public TmPacketBase {
public:
	/**
	 * This is the default constructor of the class.
	 *
	 * It sets all values to default for a CCSDS Idle Packet (i.e. APID is 2047).
	 */
	TmPacket( void );
	/**
	 * A constructor which directly sets all relevant header information.
	 * @param apid			Sets the packet's APID field.
	 * @param service		Sets the packet's Service ID field.
	 * 		This specifies the source service.
	 * @param subservice	Sets the packet's Service Subtype field.
	 * 		This specifies the source sub-service.
	 * @param packet_counter	Sets the Packet counter field of this packet.
	 */
	TmPacket( uint16_t apid, uint8_t service, uint8_t subservice, uint8_t packet_counter = 0 );
	/**
	 * This is the empty default destructor of the class.
	 */
	~TmPacket( void );
	/**
	 * With this call, application data can be added to the Source Data
	 * buffer.
	 * The Error Control Field is updated automatically.
	 * @param data	The data to add to the Source Data field.
	 * @param size	The size of the data to add.
	 * @return	@li	\c true if \c size is smaller than the Source Data buffer.
	 * 			@li \c false else.
	 */
	bool addSourceData( uint8_t*, uint32_t );
private:
	/**
	 * This structure defines the data structure of a Telemetry Packet as
	 * local data.
	 *
	 * There's a buffer which corresponds to the Telemetry Source Data Field
	 * with a maximum size of \c byte_size.
	 */
	struct TmPacketData {
		CCSDSPrimaryHeader primary;
		PUSTmDataFieldHeader data_field;
		uint8_t source_data[byte_size - sizeof(CCSDSPrimaryHeader) - sizeof(PUSTmDataFieldHeader)];
	};
	/**
	 * This is the data representation of the class.
	 */
	TmPacketData local_data;
};

#endif /* TMPACKET_H_ */
