#ifndef FSFW_TMTCPACKET_PACKETID_H
#define FSFW_TMTCPACKET_PACKETID_H

#include <cstddef>

#include "defs.h"
#include "fsfw/serialize/SerializeAdapter.h"
#include "fsfw/serialize/SerializeIF.h"

struct PacketId : public SerializeIF {
 public:
  PacketId() = default;
  /**
   * Simple wrapper for Space Packet IDs. Does not check the APID for validity
   * @param packetType_
   * @param secHeaderFlag_
   * @param apid_
   */
  PacketId(ccsds::PacketType packetType_, bool secHeaderFlag_, uint16_t apid_)
      : packetType(packetType_), secHeaderFlag(secHeaderFlag_), apid(apid_) {}

  /**
   * NOTE: If the APID has an invalid value, the invalid bits will be cut off
   * @return
   */
  [[nodiscard]] uint16_t raw() const {
    return (static_cast<uint16_t>(packetType) << 12) |
           (static_cast<uint16_t>(secHeaderFlag) << 11) | (apid & 0x7ff);
  }

  static PacketId fromRaw(uint16_t raw) {
    uint8_t highByte = (raw >> 8) & 0xff;
    uint8_t lowByte = raw & 0xff;
    return {static_cast<ccsds::PacketType>((highByte >> 4) & 0b1),
            static_cast<bool>((highByte >> 3) & 0b1),
            static_cast<uint16_t>((highByte & 0b111) << 8 | lowByte)};
  }

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override {
    uint16_t pscRaw = raw();
    return SerializeAdapter::serialize(&pscRaw, buffer, size, maxSize, streamEndianness);
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
    packetType = static_cast<ccsds::PacketType>((highByte >> 4) & 0b1);
    secHeaderFlag = (highByte >> 3) & 0b1;
    apid = static_cast<uint16_t>((highByte & 0b111) << 8 | lowByte);
    return HasReturnvaluesIF::RETURN_OK;
  }

  ccsds::PacketType packetType = ccsds::PacketType::TM;
  bool secHeaderFlag = false;
  uint16_t apid = 0;
};

#endif  // FSFW_TMTCPACKET_PACKETID_H
