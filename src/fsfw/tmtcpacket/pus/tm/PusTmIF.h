#ifndef FSFW_TMTCPACKET_PUSTMIF_H
#define FSFW_TMTCPACKET_PUSTMIF_H

#include <cstddef>
#include <cstdint>

#include "fsfw/timemanager/TimeStamperIF.h"
#include "fsfw/tmtcpacket/pus/PusIF.h"
#include "fsfw/tmtcpacket/pus/definitions.h"

class PusTmIF : public PusIF {
 public:
  ~PusTmIF() override = default;

  /**
   * Minimum length without timestamp
   */
  static constexpr size_t MIN_SEC_HEADER_LEN = 7;
  /**
   * 2 bytes for CRC16
   */
  static constexpr size_t MIN_TM_SIZE = sizeof(ccsds::PrimaryHeader) + MIN_SEC_HEADER_LEN + 2;

  virtual uint8_t getScTimeRefStatus() = 0;
  virtual uint16_t getMessageTypeCounter() = 0;
  virtual uint16_t getDestId() = 0;
  virtual TimeStamperIF* getTimestamper() = 0;
};

#endif  // FSFW_TMTCPACKET_PUSTMIF_H
