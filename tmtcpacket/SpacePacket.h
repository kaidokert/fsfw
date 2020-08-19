#ifndef SPACEPACKET_H_
#define SPACEPACKET_H_

#include "SpacePacketBase.h"

/**
 * The SpacePacket class is a representation of a simple CCSDS Space Packet
 * without (control over) a secondary header.
 * It can be instantiated with a size smaller than \c PACKET_MAX_SIZE. Its
 * main use is to serve as an idle packet in case no other packets are sent.
 * For the ECSS PUS part the TcPacket and TmPacket classes are used.
 * A pointer to \c local_data is passed to the \c SpacePacketBase parent class,
 * so the parent's methods are reachable.
 * @ingroup tmtcpackets
 */
class SpacePacket: public SpacePacketBase {
public:
	static const uint16_t PACKET_MAX_SIZE = 1024;
	/**
	 * The constructor initializes the packet and sets all header information
	 * according to the passed parameters.
	 * @param packetDataLength Sets the packet data length field and therefore specifies the size of the packet.
	 * @param isTelecommand Sets the packet type field to either TC (true) or TM (false).
	 * @param apid Sets the packet's APID field. The default value describes an idle packet.
	 * @param sequenceCount ets the packet's Source Sequence Count field.
	 */
	SpacePacket(uint16_t packetDataLength, bool isTelecommand = false,
			uint16_t apid = APID_IDLE_PACKET, uint16_t sequenceCount = 0);
	/**
	 * The class's default destructor.
	 */
	virtual ~SpacePacket();
	/**
	 * With this call, the complete data content (including the CCSDS Primary
	 * Header) is overwritten with the byte stream given.
	 * @param p_data		Pointer to data to overwrite the content with
	 * @param packet_size	Size of the data
	 * @return	@li	\c true if packet_size is smaller than \c MAX_PACKET_SIZE.
	 * 			@li \c false else.
	 */
	bool addWholeData(const uint8_t* p_data, uint32_t packet_size);
protected:
	/**
	 * This structure defines the data structure of a Space Packet as local data.
	 * There's a buffer which corresponds to the Space Packet Data Field with a
	 * maximum size of \c PACKET_MAX_SIZE.
	 */
	struct PacketStructured {
		CCSDSPrimaryHeader header;
		uint8_t buffer[PACKET_MAX_SIZE];
	};
	/**
	 * This union simplifies accessing the full data content of the Space Packet.
	 * This is achieved by putting the \c PacketStructured struct in a union with
	 * a plain buffer.
	 */
	union SpacePacketData {
		PacketStructured fields;
		uint8_t byteStream[PACKET_MAX_SIZE + sizeof(CCSDSPrimaryHeader)];
	};
	/**
	 * This is the data representation of the class.
	 * It is a struct of CCSDS Primary Header and a data field, which again is
	 * packed in an union, so the data can be accessed as a byte stream without
	 * a cast.
	 */
	SpacePacketData localData;
};

#endif /* SPACEPACKET_H_ */
