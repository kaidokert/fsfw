#ifndef FSFW_TMTCPACKET_TCPACKETSTORED_H
#define FSFW_TMTCPACKET_TCPACKETSTORED_H

#include "fsfw/tmtcpacket/PacketStorageHelper.h"

class TcPacketStored : public PacketStorageHelper {
  TcPacketStored(StorageManagerIF* store);
};

#endif  // FSFW_TMTCPACKET_TCPACKETSTORED_H
