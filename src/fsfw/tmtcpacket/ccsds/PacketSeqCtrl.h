#ifndef FSFW_TMTCPACKET_PACKETSEQCTRL_H
#define FSFW_TMTCPACKET_PACKETSEQCTRL_H

#include "defs.h"
#include "fsfw/serialize/SerializeAdapter.h"
#include "fsfw/serialize/SerializeIF.h"

struct PacketSeqCtrl : public SerializeIF {
 public:
  PacketSeqCtrl() = default;

  PacketSeqCtrl(ccsds::SequenceFlags seqFlags, uint16_t seqCount)
      : seqFlags(seqFlags), seqCount(seqCount) {}

  bool operator==(const PacketSeqCtrl &other) const {
    return seqCount == other.seqCount and seqFlags == other.seqFlags;
  }

  /**
   * NOTE: If the sequence control has an invalid value, the invalid bits will be cut off
   * @return
   */
  [[nodiscard]] uint16_t raw() const {
    return (static_cast<uint16_t>(seqFlags) << 14) | (seqCount & 0x3FFF);
  }

  static PacketSeqCtrl fromRaw(uint16_t raw) {
    uint8_t highByte = (raw >> 8) & 0xff;
    uint8_t lowByte = raw & 0xff;
    return {static_cast<ccsds::SequenceFlags>((highByte >> 6) & 0b11),
            static_cast<uint16_t>(((highByte << 8) | lowByte) & 0x3FFF)};
  }

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override {
    uint16_t packetIdRaw = raw();
    return SerializeAdapter::serialize(&packetIdRaw, buffer, size, maxSize, streamEndianness);
  }
  [[nodiscard]] size_t getSerializedSize() const override { return 2; }

  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override {
    if (*size < 2) {
      return SerializeIF::STREAM_TOO_SHORT;
    }
    uint8_t highByte = **buffer;
    *buffer += 1;
    uint8_t lowByte = **buffer;
    *buffer += 1;
    *size -= 2;
    seqFlags = static_cast<ccsds::SequenceFlags>((highByte >> 6) & 0b11);
    seqCount = ((highByte << 8) | lowByte) & 0x3FFF;
    return returnvalue::OK;
  }

  ccsds::SequenceFlags seqFlags = ccsds::SequenceFlags::CONTINUATION;
  uint16_t seqCount = 0;

  using SerializeIF::deSerialize;
  using SerializeIF::serialize;
};

#endif  // FSFW_TMTCPACKET_PACKETSEQCTRL_H
