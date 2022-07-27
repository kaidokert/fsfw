#ifndef FSFW_TMTCPACKET_SPACEPACKETREADERIF_H
#define FSFW_TMTCPACKET_SPACEPACKETREADERIF_H

#include "fsfw/tmtcpacket/ccsds/SpacePacketIF.h"

class ReadablePacketIF {
 public:
  virtual ~ReadablePacketIF() = default;

  /**
   * This method returns a raw uint8_t pointer to the readable data
   */
  [[nodiscard]] virtual const uint8_t* getFullData() const = 0;
};
#endif  // FSFW_TMTCPACKET_SPACEPACKETREADERIF_H
