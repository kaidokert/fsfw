#ifndef FSFW_TMTCPACKET_HEADER_H
#define FSFW_TMTCPACKET_HEADER_H

#include <cstddef>
#include <cstdint>

namespace ccsds {

static constexpr size_t HEADER_LEN = 6;

struct PrimaryHeader {
  uint8_t packetIdHAndVersion;
  uint8_t packetIdL;
  uint8_t packetSeqCtrlH;
  uint8_t packetSeqCtrlL;
  uint8_t packetLenH;
  uint8_t packetLenL;
};

uint16_t getPacketId(const PrimaryHeader& header);
uint16_t getPacketSeqCtrl(const PrimaryHeader& header);
uint8_t getVersion(const PrimaryHeader& header);
uint16_t getPacketLen(const PrimaryHeader& header);

void setPacketId(PrimaryHeader& header, uint16_t packetId);
void setApid(PrimaryHeader& header, uint16_t apid);
void setSequenceCount(PrimaryHeader& header, uint16_t seqCount);

}  // namespace ccsds

#endif  // FSFW_TMTCPACKET_HEADER_H
