#ifndef FSFW_TMTCPACKET_SPACEPACKETREADERIF_H
#define FSFW_TMTCPACKET_SPACEPACKETREADERIF_H

#include "SpacePacketIF.h"

class ReadablePacketIF {
 public:
  /**
   * This method returns a raw uint8_t pointer to the readable data
   */
  virtual const uint8_t* getFullData() = 0;
};
#endif  // FSFW_TMTCPACKET_SPACEPACKETREADERIF_H
