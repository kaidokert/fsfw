#ifndef FSFW_TMTCPACKET_PUSIF_H
#define FSFW_TMTCPACKET_PUSIF_H

#include <cstdint>

#include "fsfw/tmtcpacket/ccsds/SpacePacketIF.h"

class PusIF : public SpacePacketIF {
 public:
  ~PusIF() override = default;
  /**
   * This command returns the TC Packet PUS Version Number.
   * The version number of ECSS PUS 2003 is 1.
   * It consists of the second to fourth highest bits of the
   * first byte.
   * @return
   */
  [[nodiscard]] virtual uint8_t getPusVersion() const = 0;

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
   * This is a getter for a pointer to the packet's Application data.
   *
   * These are the bytes that follow after the Data Field Header. They form
   * the packet's application data.
   * @return	A pointer to the PUS Application Data.
   */
  [[nodiscard]] virtual const uint8_t* getUserData(size_t& appDataLen) const = 0;
  /**
   * This method calculates the size of the PUS Application data field.
   *
   * It takes the information stored in the CCSDS Packet Data Length field
   * and subtracts the Data Field Header size and the CRC size.
   * @return	The size of the PUS Application Data (without Error Control
   * 		field)
   */
  [[nodiscard]] virtual uint16_t getUserDataSize() const = 0;
};

#endif  // FSFW_TMTCPACKET_PUSIF_H
