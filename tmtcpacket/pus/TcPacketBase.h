#ifndef TCPACKETBASE_H_
#define TCPACKETBASE_H_

#include <framework/tmtcpacket/SpacePacketBase.h>

/**
 * This struct defines a byte-wise structured PUS TC Data Field Header.
 * Any optional fields in the header must be added or removed here.
 * Currently, the Source Id field is present with one byte.
 * @ingroup tmtcpackets
 */
struct PUSTcDataFieldHeader {
	uint8_t version_type_ack;
	uint8_t service_type;
	uint8_t service_subtype;
	uint8_t source_id;
};

/**
 * This struct defines the data structure of a PUS Telecommand Packet when
 * accessed via a pointer.
 * @ingroup tmtcpackets
 */
struct TcPacketPointer {
	CCSDSPrimaryHeader primary;
	PUSTcDataFieldHeader data_field;
	uint8_t data;
};

/**
 * This class is the basic data handler for any ECSS PUS Telecommand packet.
 *
 * In addition to \SpacePacketBase, the class provides methods to handle
 * the standardized entries of the PUS TC Packet Data Field Header.
 * It does not contain the packet data itself but a pointer to the
 * data must be set on instantiation. An invalid pointer may cause
 * damage, as no getter method checks data validity. Anyway, a NULL
 * check can be performed by making use of the getWholeData method.
 * @ingroup tmtcpackets
 */
class TcPacketBase : public SpacePacketBase {
protected:
	/**
	 * A pointer to a structure which defines the data structure of
	 * the packet's data.
	 *
	 * To be hardware-safe, all elements are of byte size.
	 */
	TcPacketPointer* tcData;
public:
	static const uint16_t TC_PACKET_MIN_SIZE = (sizeof(CCSDSPrimaryHeader) + sizeof(PUSTcDataFieldHeader) + 2);
	/**
	 * With this constant for the acknowledge field responses on all levels are expected.
	 */
	static const uint8_t ACK_ALL = 0b1111;
	/**
	 * With this constant for the acknowledge field a response on acceptance is expected.
	 */
	static const uint8_t ACK_ACCEPTANCE = 0b0001;
	/**
	 * With this constant for the acknowledge field a response on start of execution is expected.
	 */
	static const uint8_t ACK_START = 0b0010;
	/**
	 * With this constant for the acknowledge field responses on execution steps are expected.
	 */
	static const uint8_t ACK_STEP = 0b0100;
	/**
	 * With this constant for the acknowledge field a response on completion is expected.
	 */
	static const uint8_t ACK_COMPLETION = 0b1000;
	/**
	 * With this constant for the acknowledge field no responses are expected.
	 */
	static const uint8_t ACK_NONE = 0b000;
	/**
	 * This is the default constructor.
	 * It sets its internal data pointer to the address passed and also
	 * forwards the data pointer to the parent SpacePacketBase class.
	 * @param set_address	The position where the packet data lies.
	 */
	TcPacketBase( const uint8_t* set_data );
	/**
	 * This is the empty default destructor.
	 */
	virtual ~TcPacketBase();
	/**
	 * Initializes the Tc Packet header.
	 * @param apid APID used.
	 * @param service	PUS Service
	 * @param subservice PUS Subservice
	 * @param packetSubcounter Additional subcounter used.
	 */
	/**
	 * Initializes the Tc Packet header.
	 * @param apid APID used.
	 * @param sequenceCount Sequence Count in the primary header.
	 * @param ack Which acknowledeges are expected from the receiver.
	 * @param service	PUS Service
	 * @param subservice PUS Subservice
	 */
	void initializeTcPacket(uint16_t apid, uint16_t sequenceCount, uint8_t ack, uint8_t service, uint8_t subservice);
	/**
	 * This command returns the CCSDS Secondary Header Flag.
	 * It shall always be zero for PUS Packets. This is the
	 * highest bit of the first byte of the Data Field Header.
	 * @return	the CCSDS Secondary Header Flag
	 */
	uint8_t getSecondaryHeaderFlag();
	/**
	 * This command returns the TC Packet PUS Version Number.
	 * The version number of ECSS PUS 2003 is 1.
	 * It consists of the second to fourth highest bits of the
	 * first byte.
	 * @return
	 */
	uint8_t getPusVersionNumber();
	/**
	 * This is a getter for the packet's Ack field, which are the lowest four
	 * bits of the first byte of the Data Field Header.
	 *
	 * It is packed in a uint8_t variable.
	 * @return	The packet's PUS Ack field.
	 */
	uint8_t getAcknowledgeFlags();
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
	 * This is a getter for a pointer to the packet's Application data.
	 *
	 * These are the bytes that follow after the Data Field Header. They form
	 * the packet's application data.
	 * @return	A pointer to the PUS Application Data.
	 */
	const uint8_t* getApplicationData() const;
	/**
	 * This method calculates the size of the PUS Application data field.
	 *
	 * It takes the information stored in the CCSDS Packet Data Length field
	 * and subtracts the Data Field Header size and the CRC size.
	 * @return	The size of the PUS Application Data (without Error Control
	 * 		field)
	 */
	uint16_t getApplicationDataSize();
	/**
	 * This getter returns the Error Control Field of the packet.
	 *
	 * The field is placed after any possible Application Data. If no
	 * Application Data is present there's still an Error Control field. It is
	 * supposed to be a 16bit-CRC.
	 * @return	The PUS Error Control
	 */
	uint16_t getErrorControl();
	/**
	 * With this method, the Error Control Field is updated to match the
	 * current content of the packet.
	 */
	void setErrorControl();
	/**
	 * With this method, the packet data pointer can be redirected to another
	 * location.
	 *
	 * This call overwrites the parent's setData method to set both its
	 * \c tc_data pointer and the parent's \c data pointer.
	 *
	 * @param p_data	A pointer to another PUS Telecommand Packet.
	 */
	void setData( const uint8_t* p_data );
	/**
	 * This is a debugging helper method that prints the whole packet content
	 * to the screen.
	 */
	void print();
};


#endif /* TCPACKETBASE_H_ */
