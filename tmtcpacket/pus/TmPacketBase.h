#ifndef TMTCPACKET_PUS_TMPACKETBASE_H_
#define TMTCPACKET_PUS_TMPACKETBASE_H_

#include "../SpacePacketBase.h"
#include "../../timemanager/TimeStamperIF.h"
#include "../../timemanager/Clock.h"
#include "../../objectmanager/SystemObjectIF.h"

namespace Factory{
void setStaticFrameworkObjectIds();
}

/**
 * This struct defines a byte-wise structured PUS TM Data Field Header.
 * Any optional fields in the header must be added or removed here.
 * Currently, no Destination field is present, but an eigth-byte representation
 * for a time tag.
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

/**
 * This class is the basic data handler for any ECSS PUS Telemetry packet.
 *
 * In addition to #SpacePacketBase, the class provides methods to handle
 * the standardized entries of the PUS TM Packet Data Field Header.
 * It does not contain the packet data itself but a pointer to the
 * data must be set on instantiation. An invalid pointer may cause
 * damage, as no getter method checks data validity. Anyway, a NULL
 * check can be performed by making use of the getWholeData method.
 * @ingroup tmtcpackets
 */
class TmPacketBase : public SpacePacketBase {
	friend void (Factory::setStaticFrameworkObjectIds)();
public:
	/**
	 * This constant defines the minimum size of a valid PUS Telemetry Packet.
	 */
	static const uint32_t  TM_PACKET_MIN_SIZE = (sizeof(CCSDSPrimaryHeader) +
	        sizeof(PUSTmDataFieldHeader) + 2);
	//! Maximum size of a TM Packet in this mission.
	//! TODO: Make this dependant on a config variable.
	static const uint32_t MISSION_TM_PACKET_MAX_SIZE = 2048;
	//! First byte of secondary header for PUS-A packets.
	//! TODO: Maybe also support PUS-C via config?
	static const uint8_t VERSION_NUMBER_BYTE_PUS_A = 0b00010000;

	/**
	 * This is the default constructor.
	 * It sets its internal data pointer to the address passed and also
	 * forwards the data pointer to the parent SpacePacketBase class.
	 * @param set_address	The position where the packet data lies.
	 */
	TmPacketBase( uint8_t* setData );
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
     * With this method, the Error Control Field is updated to match the
     * current content of the packet. This method is not protected because
     * a recalculation by the user might be necessary when manipulating fields
     * like the sequence count.
     */
    void setErrorControl();

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
	 * This is a debugging helper method that prints the whole packet content
	 * to the screen.
	 */
	void print();
	/**
	 * Interprets the "time"-field in the secondary header and returns it in
	 * timeval format.
	 * @return Converted timestamp of packet.
	 */
	ReturnValue_t getPacketTime(timeval* timestamp) const;
	/**
	 * Returns a raw pointer to the beginning of the time field.
	 * @return Raw pointer to time field.
	 */
	uint8_t* getPacketTimeRaw() const;

	size_t getTimestampSize() const;

protected:
	/**
	 * A pointer to a structure which defines the data structure of
	 * the packet's data.
	 *
	 * To be hardware-safe, all elements are of byte size.
	 */
	TmPacketPointer* tmData;
	/**
	 * The timeStamper is responsible for adding a timestamp to the packet.
	 * It is initialized lazy.
	 */
	static TimeStamperIF* timeStamper;
	//! The ID to use when looking for a time stamper.
	static object_id_t timeStamperId;

    /**
     * Initializes the Tm Packet header.
     * Does set the timestamp (to now), but not the error control field.
     * @param apid APID used.
     * @param service   PUS Service
     * @param subservice PUS Subservice
     * @param packetSubcounter Additional subcounter used.
     */
    void initializeTmPacket(uint16_t apid, uint8_t service, uint8_t subservice,
            uint8_t packetSubcounter);

    /**
     * With this method, the packet data pointer can be redirected to another
     * location.
     *
     * This call overwrites the parent's setData method to set both its
     * @c tc_data pointer and the parent's @c data pointer.
     *
     * @param p_data    A pointer to another PUS Telemetry Packet.
     */
    void setData( const uint8_t* pData );

    /**
     * In case data was filled manually (almost never the case).
     * @param size Size of source data (without CRC and data filed header!).
     */
    void setSourceDataSize(uint16_t size);

    /**
     * Checks if a time stamper is available and tries to set it if not.
     * @return Returns false if setting failed.
     */
    bool checkAndSetStamper();
};


#endif /* TMTCPACKET_PUS_TMPACKETBASE_H_ */
