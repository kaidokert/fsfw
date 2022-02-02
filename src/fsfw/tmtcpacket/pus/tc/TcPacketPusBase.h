#ifndef TMTCPACKET_PUS_TCPACKETBASE_H_
#define TMTCPACKET_PUS_TCPACKETBASE_H_

#include <fsfw/tmtcpacket/RedirectableDataPointerIF.h>
#include "fsfw/tmtcpacket/SpacePacketBase.h"
#include <cstddef>

/**
 * This class is the basic data handler for any ECSS PUS Telecommand packet.
 *
 * In addition to #SpacePacketBase, the class provides methods to handle
 * the standardized entries of the PUS TC Packet Data Field Header.
 * It does not contain the packet data itself but a pointer to the
 * data must be set on instantiation. An invalid pointer may cause
 * damage, as no getter method checks data validity. Anyway, a NULL
 * check can be performed by making use of the getWholeData method.
 * @ingroup tmtcpackets
 */
class TcPacketPusBase : public SpacePacketBase,
    virtual public RedirectableDataPointerIF {
    friend class TcPacketStoredBase;
public:

    enum AckField {
        //! No acknowledgements are expected.
        ACK_NONE = 0b0000,
        //! Acknowledgements on acceptance are expected.
        ACK_ACCEPTANCE = 0b0001,
        //! Acknowledgements on start are expected.
        ACK_START = 0b0010,
        //! Acknowledgements on step are expected.
        ACK_STEP = 0b0100,
        //! Acknowledfgement on completion are expected.
        ACK_COMPLETION = 0b1000
    };

    static constexpr uint8_t ACK_ALL = ACK_ACCEPTANCE | ACK_START | ACK_STEP |
            ACK_COMPLETION;

    /**
     * This is the default constructor.
     * It sets its internal data pointer to the address passed and also
     * forwards the data pointer to the parent SpacePacketBase class.
     * @param setData	The position where the packet data lies.
     */
    TcPacketPusBase( const uint8_t* setData );
    /**
     * This is the empty default destructor.
     */
    virtual ~TcPacketPusBase();

    /**
     * This command returns the CCSDS Secondary Header Flag.
     * It shall always be zero for PUS Packets. This is the
     * highest bit of the first byte of the Data Field Header.
     * @return	the CCSDS Secondary Header Flag
     */
    virtual uint8_t getSecondaryHeaderFlag() const = 0;
    /**
     * This command returns the TC Packet PUS Version Number.
     * The version number of ECSS PUS 2003 is 1.
     * It consists of the second to fourth highest bits of the
     * first byte.
     * @return
     */
    virtual uint8_t getPusVersionNumber() const = 0;
    /**
     * This is a getter for the packet's Ack field, which are the lowest four
     * bits of the first byte of the Data Field Header.
     *
     * It is packed in a uint8_t variable.
     * @return	The packet's PUS Ack field.
     */
    virtual uint8_t getAcknowledgeFlags() const = 0;
    /**
     * This is a getter for the packet's PUS Service ID, which is the second
     * byte of the Data Field Header.
     * @return	The packet's PUS Service ID.
     */
    virtual uint8_t getService() const = 0;
    /**
     * This is a getter for the packet's PUS Service Subtype, which is the
     * third byte of the Data Field Header.
     * @return	The packet's PUS Service Subtype.
     */
    virtual uint8_t getSubService() const = 0;
    /**
     * The source ID can be used to have an additional identifier, e.g. for different ground
     * station.
     * @return
     */
    virtual uint16_t getSourceId() const = 0;

    /**
     * This is a getter for a pointer to the packet's Application data.
     *
     * These are the bytes that follow after the Data Field Header. They form
     * the packet's application data.
     * @return	A pointer to the PUS Application Data.
     */
    virtual const uint8_t* getApplicationData() const = 0;
    /**
     * This method calculates the size of the PUS Application data field.
     *
     * It takes the information stored in the CCSDS Packet Data Length field
     * and subtracts the Data Field Header size and the CRC size.
     * @return	The size of the PUS Application Data (without Error Control
     * 		field)
     */
    virtual uint16_t getApplicationDataSize() const = 0;
    /**
     * This getter returns the Error Control Field of the packet.
     *
     * The field is placed after any possible Application Data. If no
     * Application Data is present there's still an Error Control field. It is
     * supposed to be a 16bit-CRC.
     * @return	The PUS Error Control
     */
    virtual uint16_t getErrorControl() const = 0;
    /**
     * With this method, the Error Control Field is updated to match the
     * current content of the packet.
     */
    virtual void setErrorControl() = 0;

    /**
     * Calculate full packet length from application data length.
     * @param appDataLen
     * @return
     */
    virtual size_t calculateFullPacketLength(size_t appDataLen) const = 0;

    /**
     * This is a debugging helper method that prints the whole packet content
     * to the screen.
     */
    void print();

protected:

    /**
     * With this method, the packet data pointer can be redirected to another
     * location.
     * This call overwrites the parent's setData method to set both its
     * @c tc_data pointer and the parent's @c data pointer.
     *
     * @param p_data    A pointer to another PUS Telecommand Packet.
     */
    virtual ReturnValue_t setData(uint8_t* pData, size_t maxSize,
            void* args = nullptr) override = 0;
};


#endif /* TMTCPACKET_PUS_TCPACKETBASE_H_ */
