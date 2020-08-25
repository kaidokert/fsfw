#ifndef FRAMEWORK_TMTCPACKET_PUS_TMPACKETMINIMAL_H_
#define FRAMEWORK_TMTCPACKET_PUS_TMPACKETMINIMAL_H_


#include "../../tmtcpacket/SpacePacketBase.h"
#include "../../returnvalues/HasReturnvaluesIF.h"

struct timeval;
class PacketTimestampInterpreterIF;
/**
 * This is a minimal version of a PUS TmPacket without any variable field, or,
 * in other words with Service Type, Subtype and subcounter only.
 * This is required for handling TM packets with different APIDs with different
 * secondary headers.
 */
class TmPacketMinimal : public SpacePacketBase {
public:
	/**
	 * This is the default constructor.
	 * It sets its internal data pointer to the address passed and also
	 * forwards the data pointer to the parent SpacePacketBase class.
	 * @param set_address	The position where the packet data lies.
	 */
	TmPacketMinimal( const uint8_t* set_data );
	/**
	 * This is the empty default destructor.
	 */
	virtual ~TmPacketMinimal();
	/**
	 * This is a getter for the packet's PUS Service ID, which is the second
	 * byte of the Data Field Header.
	 * @return	The packet's PUS Service ID.
	 */
	uint8_t getService();
	/**
	 * This is a getter for the packet's PUS Service Subtype, which is the
	 * third byte of the Data Field Header.
	 * @return	The packet's PUS Service Subtype.
	 */
	uint8_t getSubService();
	/**
	 * Returns the subcounter.
	 * @return the subcounter of the Data Field Header.
	 */
	uint8_t getPacketSubcounter();
	struct PUSTmMinimalHeader {
		uint8_t version_type_ack;
		uint8_t service_type;
		uint8_t service_subtype;
		uint8_t subcounter;
	};

	ReturnValue_t getPacketTime(timeval* timestamp);

	ReturnValue_t getPacketTimeRaw(const uint8_t** timePtr, uint32_t* size);

	static void setInterpretTimestampObject(PacketTimestampInterpreterIF* interpreter);
	/**
	 * This struct defines the data structure of a PUS Telecommand Packet when
	 * accessed via a pointer.
	 * @ingroup tmtcpackets
	 */
	struct TmPacketMinimalPointer {
		CCSDSPrimaryHeader primary;
		PUSTmMinimalHeader data_field;
		uint8_t rest;
	};
	//Must include a checksum and is therefore at least one larger than the above struct.
	static const uint16_t MINIMUM_SIZE = sizeof(TmPacketMinimalPointer) +1;
protected:
	/**
	 * A pointer to a structure which defines the data structure of
	 * the packet's data.
	 *
	 * To be hardware-safe, all elements are of byte size.
	 */
	TmPacketMinimalPointer* tm_data;

	static PacketTimestampInterpreterIF* timestampInterpreter;
};



#endif /* FRAMEWORK_TMTCPACKET_PUS_TMPACKETMINIMAL_H_ */
