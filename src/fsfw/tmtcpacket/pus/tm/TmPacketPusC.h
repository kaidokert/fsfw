#ifndef FSFW_TMTCPACKET_PUS_TMPACKETPUSC_H_
#define FSFW_TMTCPACKET_PUS_TMPACKETPUSC_H_

#include "TmPacketBase.h"
#include "fsfw/tmtcpacket/SpacePacketBase.h"
#include "fsfw/timemanager/TimeStamperIF.h"
#include "fsfw/timemanager/Clock.h"
#include "fsfw/objectmanager/SystemObjectIF.h"

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
struct PUSTmDataFieldHeaderPusC {
    uint8_t versionTimeReferenceField;
    uint8_t serviceType;
    uint8_t serviceSubtype;
    uint8_t subcounterMsb;
    uint8_t subcounterLsb;
    uint8_t destinationIdMsb;
    uint8_t destinationIdLsb;
    uint8_t time[TimeStamperIF::MISSION_TIMESTAMP_SIZE];
};

/**
 * This struct defines the data structure of a PUS Telecommand Packet when
 * accessed via a pointer.
 * @ingroup tmtcpackets
 */
struct TmPacketPointerPusC {
    CCSDSPrimaryHeader primary;
    PUSTmDataFieldHeaderPusC dataField;
    uint8_t data;
};

/**
 * PUS A packet implementation
 * @ingroup tmtcpackets
 */
class TmPacketPusC: public TmPacketBase {
    friend void (Factory::setStaticFrameworkObjectIds)();
public:
    /**
     * This constant defines the minimum size of a valid PUS Telemetry Packet.
     */
    static const uint32_t  TM_PACKET_MIN_SIZE = (sizeof(CCSDSPrimaryHeader) +
            sizeof(PUSTmDataFieldHeaderPusC) + 2);
    //! Maximum size of a TM Packet in this mission.
    static const uint32_t MISSION_TM_PACKET_MAX_SIZE = fsfwconfig::FSFW_MAX_TM_PACKET_SIZE;

    /**
     * This is the default constructor.
     * It sets its internal data pointer to the address passed and also
     * forwards the data pointer to the parent SpacePacketBase class.
     * @param set_address   The position where the packet data lies.
     */
    TmPacketPusC( uint8_t* setData );
    /**
     * This is the empty default destructor.
     */
    virtual ~TmPacketPusC();

    /* TmPacketBase implementations */
    uint8_t getService() override;
    uint8_t getSubService() override;
    uint8_t* getSourceData() override;
    uint16_t getSourceDataSize() override;
    uint16_t getDataFieldSize() override;

    /**
     * Returns a raw pointer to the beginning of the time field.
     * @return Raw pointer to time field.
     */
    uint8_t* getPacketTimeRaw() const override;
    size_t getTimestampSize() const override;

    size_t getPacketMinimumSize() const override;

protected:
    /**
     * A pointer to a structure which defines the data structure of
     * the packet's data.
     *
     * To be hardware-safe, all elements are of byte size.
     */
    TmPacketPointerPusC* tmData;

    /**
     * Initializes the Tm Packet header.
     * Does set the timestamp (to now), but not the error control field.
     * @param apid APID used.
     * @param service   PUS Service
     * @param subservice PUS Subservice
     * @param packetSubcounter Additional subcounter used.
     */
    ReturnValue_t initializeTmPacket(uint16_t apid, uint8_t service, uint8_t subservice,
            uint16_t packetSubcounter, uint16_t destinationId = 0, uint8_t timeRefField = 0);

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

};

#endif /* FSFW_TMTCPACKET_PUS_TMPACKETPUSC_H_ */
