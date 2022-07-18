#ifndef FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H
#define FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H

#include "fsfw/tmtcpacket/RedirectableDataPointerIF.h"
#include "fsfw/tmtcpacket/SpacePacketIF.h"

class PusTcCreator : public SpacePacketIF {
 public:
  PusTcCreator(const uint8_t* data, size_t maxSize);

 private:
};

#endif  // FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H
