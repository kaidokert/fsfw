#ifndef TCPACKET_H_
#define TCPACKET_H_

#include <framework/tmtcpacket/pus/TcPacketBase.h>

/**
 * This constant defines the minimum size of a valid PUS Telecommand Packet.
 */
#define TC_PACKET_MIN_SIZE (sizeof(CCSDSPrimaryHeader) + sizeof(PUSTcDataFieldHeader) + 2)

/**
 * The TcPacket class is a representation of a ECSS PUS Telecommand packet.

 * The template parameter is used to instantiate the class with different
 * maximum Application Data sizes (to avoid wasting stack size).
 * Telecommand packets are mainly created on ground, which are accessed with
 * the TcPacketBase class. However, there are occasions when command packets
 * are created on-board, for example when commanding another PUS compatible
 * device such as the star trackers.
 * These packets can be created with the TcPacket class.
 * A pointer to \c local_data is passed to the \c TcPacketBase parent class,
 * so the parent's methods are reachable.
 * @t_param	byte_size	The maximum size of the complete packet (including CRC
 * 		and headers)
 * @ingroup tmtcpackets
 */
template<uint32_t byte_size>
class TcPacket : public TcPacketBase {
private:
	/**
	 * This structure defines the data structure of a Telecommand Packet as
	 * local data.
	 *
	 * There's a buffer which corresponds to the Telecommand Application Data
	 * Field with a maximum size of \c byte_size.
	 */
	struct TcPacketData {
		CCSDSPrimaryHeader primary;
		PUSTcDataFieldHeader data_field;
		uint8_t application_data[byte_size - sizeof(CCSDSPrimaryHeader) - sizeof(PUSTcDataFieldHeader)];
	};
	/**
	 * This is the data representation of the class.
	 */
	TcPacketData local_data;
public:
	/**
	 * This is the default constructor of the class.
	 *
	 * It sets all values to default for a CCSDS Idle Packet (i.e. APID is 2047).
	 */
	TcPacket( void );
	/**
	 * A constructor which directly sets all relevant header information.
	 * @param apid			Sets the packet's APID field.
	 * @param ack			Set's the packet's Ack field,
	 * 		which specifies number and size of verification packets returned
	 * 		for this command.
	 * @param service		Sets the packet's Service ID field.
	 * 		This specifies the destination service.
	 * @param subservice	Sets the packet's Service Subtype field.
	 * 		This specifies the destination sub-service.
	 * @param sequence_count	Sets the packet's Source Sequence Count field.
	 */
	TcPacket( uint16_t apid, uint8_t ack, uint8_t service, uint8_t subservice, uint16_t sequence_count = 0 );
	/**
	 * This is the empty default destructor of the class.
	 */
	~TcPacket( void );
	/**
	 * With this call, application data can be added to the Application Data
	 * buffer.
	 * The Error Control Field is updated automatically.
	 * @param data	The data to add to the Application Data field.
	 * @param size	The size of the data to add.
	 * @return	@li	\c true if \c size is smaller than the Application Data
	 * 		buffer.
	 * 			@li \c false else.
	 */
	bool addApplicationData( uint8_t* data, uint32_t size );
};

#endif /* TCPACKET_H_ */
