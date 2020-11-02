#ifndef FSFW_TMTCPACKET_SPACEPACKETBASE_H_
#define FSFW_TMTCPACKET_SPACEPACKETBASE_H_

#include "ccsds_header.h"
#include <cstddef>

/**
 * @defgroup tmtcpackets Space Packets
 * This is the group, where all classes associated with Telecommand and
 * Telemetry packets belong to.
 * The class hierarchy resembles the dependency between the different standards
 * applied, namely the CCSDS Space Packet standard and the ECCSS Packet
 * Utilization Standard. Most field and structure names are taken from these
 * standards.
 */

/**
 * This struct defines the data structure of a Space Packet when accessed
 * via a pointer.
 * @ingroup tmtcpackets
 */
struct SpacePacketPointer {
	CCSDSPrimaryHeader header;
	uint8_t packet_data;
};

/**
 * This class is the basic data handler for any CCSDS Space Packet
 * compatible Telecommand and Telemetry packet.
 * It does not contain the packet data itself but a pointer to the
 * data must be set on instantiation. An invalid pointer may cause
 * damage, as no getter method checks data validity. Anyway, a NULL
 * check can be performed by making use of the getWholeData method.
 * Remark: All bit numbers in this documentation are counted from
 * the most significant bit (from left).
 * @ingroup tmtcpackets
 */
class SpacePacketBase {
protected:
	/**
	 * A pointer to a structure which defines the data structure of
	 * the packet header.
	 * To be hardware-safe, all elements are of byte size.
	 */
	SpacePacketPointer* data;
public:
	static const uint16_t LIMIT_APID = 2048; //2^1
	static const uint16_t LIMIT_SEQUENCE_COUNT = 16384; // 2^14
	static const uint16_t APID_IDLE_PACKET = 0x7FF;
	static const uint8_t TELECOMMAND_PACKET = 1;
	static const uint8_t TELEMETRY_PACKET = 0;
	/**
	 * This definition defines the CRC size in byte.
	 */
	static const uint8_t CRC_SIZE = 2;
	/**
	 * This is the minimum size of a SpacePacket.
	 */
	static const uint16_t MINIMUM_SIZE = sizeof(CCSDSPrimaryHeader) + CRC_SIZE;
	/**
	 * This is the default constructor.
	 * It sets its internal data pointer to the address passed.
	 * @param set_address	The position where the packet data lies.
	 */
	SpacePacketBase( const uint8_t* set_address );
	/**
	 * No data is allocated, so the destructor is empty.
	 */
	virtual ~SpacePacketBase();

	//CCSDS Methods:
	/**
	 * Getter for the packet version number field.
	 * @return Returns the highest three bit of the packet in one byte.
	 */
	uint8_t getPacketVersionNumber( void );
	/**
	 * This method checks the type field in the header.
	 * This bit specifies, if the command is interpreted as Telecommand of
	 * as Telemetry. For a Telecommand, the bit is set.
	 * @return Returns true if the bit is set and false if not.
	 */
	bool isTelecommand( void );

	void initSpacePacketHeader(bool isTelecommand, bool hasSecondaryHeader,
	        uint16_t apid, uint16_t sequenceCount = 0);
	/**
	 * The CCSDS header provides a secondary header flag (the fifth-highest bit),
	 *  which is checked with this method.
	 * @return	Returns true if the bit is set and false if not.
	 */
	bool hasSecondaryHeader( void );
	/**
	 * Returns the complete first two bytes of the packet, which together form
	 * the CCSDS packet id.
	 * @return	The CCSDS packet id.
	 */
	uint16_t getPacketId( void );
	/**
	 * Returns the APID of a packet, which are the lowest 11 bit of the packet
	 * id.
	 * @return The CCSDS APID.
	 */
	uint16_t getAPID( void ) const;
	/**
	 * Sets the APID of a packet, which are the lowest 11 bit of the packet
	 * id.
	 * @param 	The APID to set. The highest five bits of the parameter are
	 * 			ignored.
	 */
	void setAPID( uint16_t setAPID );
	/**
	 * Returns the CCSDS packet sequence control field, which are the third and
	 * the fourth byte of the CCSDS primary header.
	 * @return The CCSDS packet sequence control field.
	 */
	uint16_t getPacketSequenceControl( void );
	/**
	 * Returns the SequenceFlags, which are the highest two bit of the packet
	 * sequence control field.
	 * @return	The CCSDS sequence flags.
	 */
	uint8_t getSequenceFlags( void );
	/**
	 * Returns the packet sequence count, which are the lowest 14 bit of the
	 * packet sequence control field.
	 * @return The CCSDS sequence count.
	 */
	uint16_t getPacketSequenceCount( void ) const;
	/**
	 * Sets the packet sequence count, which are the lowest 14 bit of the
	 * packet sequence control field.
	 * setCount is modulo-divided by \c LIMIT_SEQUENCE_COUNT to avoid overflows.
	 * @param setCount	The value to set the count to.
	 */
	void setPacketSequenceCount( uint16_t setCount );
	/**
	 * Returns the packet data length, which is the fifth and sixth byte of the
	 * CCSDS Primary Header. The packet data length is the size of every kind
	 * of data \b after the CCSDS Primary Header \b -1.
	 * @return The CCSDS packet data length.
	 */
	uint16_t getPacketDataLength( void ); //uint16_t is sufficient, because this is limit in CCSDS standard
	/**
	 * Sets the packet data length, which is the fifth and sixth byte of the
	 * CCSDS Primary Header.
	 * @param setLength The value of the length to set. It must fit the true
	 * 					CCSDS packet data length . The packet data length is
	 * 					the size of every kind of data \b after the CCSDS
	 * 					Primary Header \b -1.
	 */
	void setPacketDataLength( uint16_t setLength );

	//Helper methods:
	/**
	 * This method returns a raw uint8_t pointer to the packet.
	 * @return	A \c uint8_t pointer to the first byte of the CCSDS primary header.
	 */
	virtual uint8_t* getWholeData( void );

	uint8_t* getPacketData();
	/**
	 * With this method, the packet data pointer can be redirected to another
	 * location.
	 * @param p_Data A pointer to another raw Space Packet.
	 */
	virtual void setData( const uint8_t* p_Data );
	/**
	 * This method returns the full raw packet size.
	 * @return	The full size of the packet in bytes.
	 */
	size_t getFullSize();

	uint32_t getApidAndSequenceCount() const;

};

#endif /* FSFW_TMTCPACKET_SPACEPACKETBASE_H_ */
