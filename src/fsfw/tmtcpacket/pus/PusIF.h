#ifndef FSFW_TMTCPACKET_PUSIF_H
#define FSFW_TMTCPACKET_PUSIF_H

#include <cstdint>

#include "fsfw/returnvalues/FwClassIds.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketIF.h"

class PusIF : public SpacePacketIF {
 public:
  static constexpr uint8_t INTERFACE_ID = CLASS_ID::PUS_IF;
  static constexpr ReturnValue_t INVALID_PUS_VERSION = returnvalue::makeCode(INTERFACE_ID, 0);
  static constexpr ReturnValue_t INVALID_CRC_16 = returnvalue::makeCode(INTERFACE_ID, 1);

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
};

#endif  // FSFW_TMTCPACKET_PUSIF_H
