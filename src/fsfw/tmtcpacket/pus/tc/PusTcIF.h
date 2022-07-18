#ifndef FSFW_TMTCPACKET_PUSTCIF_H
#define FSFW_TMTCPACKET_PUSTCIF_H

#include <cstdint>

class PusTcIF {
  public:
   virtual ~PusTcIF() = default;

   /**
   * This command returns the CCSDS Secondary Header Flag.
   * It shall always be zero for PUS Packets. This is the
   * highest bit of the first byte of the Data Field Header.
   * @return	the CCSDS Secondary Header Flag
    */
   [[nodiscard]] virtual uint8_t getSecondaryHeaderFlag() const = 0;
   /**
   * This command returns the TC Packet PUS Version Number.
   * The version number of ECSS PUS 2003 is 1.
   * It consists of the second to fourth highest bits of the
   * first byte.
   * @return
    */
   [[nodiscard]] virtual uint8_t getPusVersion() const = 0;
   /**
   * This is a getter for the packet's Ack field, which are the lowest four
   * bits of the first byte of the Data Field Header.
   *
   * It is packed in a uint8_t variable.
   * @return	The packet's PUS Ack field.
    */
   [[nodiscard]] virtual uint8_t getAcknowledgeFlags() const = 0;
   /**
   * This is a getter for the packet's PUS Service ID, which is the second
   * byte of the Data Field Header.
   * @return	The packet's PUS Service ID.
    */
   [[nodiscard]] virtual uint8_t getService() const = 0;
   /**
   * This is a getter for the packet's PUS Service Subtype, which is the
   * third byte of the Data Field Header.
   * @return	The packet's PUS Service Subtype.
    */
   [[nodiscard]] virtual uint8_t getSubService() const = 0;
   /**
   * The source ID can be used to have an additional identifier, e.g. for different ground
   * station.
   * @return
    */
   [[nodiscard]] virtual uint16_t getSourceId() const = 0;

   /**
   * This is a getter for a pointer to the packet's Application data.
   *
   * These are the bytes that follow after the Data Field Header. They form
   * the packet's application data.
   * @return	A pointer to the PUS Application Data.
    */
   [[nodiscard]] virtual const uint8_t* getApplicationData() const = 0;
   /**
   * This method calculates the size of the PUS Application data field.
   *
   * It takes the information stored in the CCSDS Packet Data Length field
   * and subtracts the Data Field Header size and the CRC size.
   * @return	The size of the PUS Application Data (without Error Control
   * 		field)
    */
   [[nodiscard]] virtual uint16_t getApplicationDataSize() const = 0;
   /**
   * This getter returns the Error Control Field of the packet.
   *
   * The field is placed after any possible Application Data. If no
   * Application Data is present there's still an Error Control field. It is
   * supposed to be a 16bit-CRC.
   * @return	The PUS Error Control
    */
   [[nodiscard]] virtual uint16_t getErrorControl() const = 0;
 };
#endif  // FSFW_TMTCPACKET_PUSTCIF_H
