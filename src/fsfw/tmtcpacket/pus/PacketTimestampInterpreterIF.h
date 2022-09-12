#ifndef FSFW_TMTCPACKET_PUS_PACKETTIMESTAMPINTERPRETERIF_H_
#define FSFW_TMTCPACKET_PUS_PACKETTIMESTAMPINTERPRETERIF_H_

#include "fsfw/returnvalues/returnvalue.h"

class PusTmMinimal;

class PacketTimestampInterpreterIF {
 public:
  virtual ~PacketTimestampInterpreterIF() {}
  virtual ReturnValue_t getPacketTime(PusTmMinimal* packet, timeval* timestamp) const = 0;
  virtual ReturnValue_t getPacketTimeRaw(PusTmMinimal* packet, const uint8_t** timePtr,
                                         uint32_t* size) const = 0;
};

#endif /* FSFW_TMTCPACKET_PUS_PACKETTIMESTAMPINTERPRETERIF_H_ */
