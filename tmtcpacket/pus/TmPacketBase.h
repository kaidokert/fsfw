#ifndef TMPACKETBASE_H_
#define TMPACKETBASE_H_

#include <framework/timemanager/TimeStamperIF.h>
#include <framework/tmtcpacket/SpacePacketBase.h>

/**
 * This struct defines a byte-wise structured PUS TM Data Field Header.
 * Any optional fields in the header must be added or removed here.
 * Currently, no Destination field is present, but an eigth-byte representation
 * for a time tag [TBD].
 * @ingroup tmtcpackets
 */
struct PUSTmDataFieldHeader {
	uint8_t version_type_ack;
	uint8_t service_type;
	uint8_t service_subtype;
	uint8_t subcounter;
//	uint8_t destination;
	uint8_t time[TimeStamperIF::MISSION_TIMESTAMP_SIZE];
};

/**
 * This struct defines the data structure of a PUS Telecommand Packet when
 * accessed via a pointer.
 * @ingroup tmtcpackets
 */
struct TmPacketPointer {
	CCSDSPrimaryHeader primary;
	PUSTmDataFieldHeader data_field;
	uint8_t data;
};

//TODO: add getTime, getSubcounter, getDestionation (if required)

/**
 * This class is the basic data handler for any ECSS PUS Telemetry packet.
 *
 * In addition to \SpacePacketBase, the class provides methods to handle
 * the standardized entries of the PUS TM Packet Data Field Header.
 * It does not contain the packet data itself but a pointer to the
 * data must be set on instantiation. An invalid pointer may cause
 * damage, as no getter method checks data validity. Anyway, a NULL
 * check can be performed by making use of the getWholeData method.
 * @ingroup tmtcpackets
 */
class TmPacketBase : public SpacePacketBase {
public:
	/**
	 * This is the default constructor.
	 * It sets its internal data pointer to the address passed and also
	 * forwards the data pointer to the parent SpacePacketBase class.
	 * @param set_address	The position where the packet data lies.
	 */
	TmPacketBase( uint8_t* set_data );
	/**
	 * This is the empty default destructor.
	 */
	virtual ~TmPacketBase();
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
	 * This is a getter for a pointer to the packet's Source data.
	 *
	 * These are the bytes that follow after the Data Field Header. They form
	 * the packet's source data.
	 * @return	A pointer to the PUS Source Data.
	 */
	uint8_t* getSourceData();
	/**
	 * This method calculates the size of the PUS Source data field.
	 *
	 * It takes the information stored in the CCSDS Packet Data Length field
	 * and subtracts the Data Field Header size and the CRC size.
	 * @return	The size of the PUS Source Data (without Error Control field)
	 */
	uint16_t getSourceDataSize();
	/**
	 * This getter returns the Error Control Field of the packet.
	 *
	 * The field is placed after any possible Source Data. If no
	 * Source Data is present there's still an Error Control field. It is
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
	 * @param p_data	A pointer to another PUS Telemetry Packet.
	 */
	void setData( const uint8_t* p_Data );
	/**
	 * This is a debugging helper method that prints the whole packet content
	 * to the screen.
	 */
	void print();
	/**
	 * This constant defines the minimum size of a valid PUS Telemetry Packet.
	 */
	static const uint32_t  TM_PACKET_MIN_SIZE = (sizeof(CCSDSPrimaryHeader) + sizeof(PUSTmDataFieldHeader) + 2);
	static const uint32_t MISSION_TM_PACKET_MAX_SIZE = 2048; //!< Maximum size of a TM Packet in this mission.
protected:
	/**
	 * A pointer to a structure which defines the data structure of
	 * the packet's data.
	 *
	 * To be hardware-safe, all elements are of byte size.
	 */
	TmPacketPointer* tm_data;
	/**
	 * The timeStamper is responsible for adding a timestamp to the packet.
	 * It is initialized lazy.
	 */
	static TimeStamperIF* timeStamper;
	/**
	 * Checks if a time stamper is available and tries to set it if not.
	 * @return Returns false if setting failed.
	 */
	bool checkAndSetStamper();
};


#endif /* TMPACKETBASE_H_ */
