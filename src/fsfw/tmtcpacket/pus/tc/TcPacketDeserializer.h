#ifndef FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H
#define FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H

#include "fsfw/tmtcpacket/RedirectableDataPointerIF.h"
#include "fsfw/tmtcpacket/SpacePacketIF.h"

class TcPacketDeserializer : public SpacePacketIF, public RedirectableDataPointerIF {
 public:
  TcPacketDeserializer(const uint8_t* data, size_t maxSize);

 private:
};

#endif  // FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H
