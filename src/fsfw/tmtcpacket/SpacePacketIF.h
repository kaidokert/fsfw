#ifndef FSFW_TMTCPACKET_SPACEPACKETIF_H
#define FSFW_TMTCPACKET_SPACEPACKETIF_H

#include <cstdint>

#include "ccsds_header.h"

namespace ccsds {

enum PacketType : uint8_t { TM = 0, TC = 1 };

enum SequenceFlags : uint8_t {
  CONTINUATION = 0b00,
  FIRST_SEGMENT = 0b01,
  LAST_SEGMENT = 0b10,
  UNSEGMENTED = 0b11
};

static const uint16_t LIMIT_APID = 2048;             // 2^11
static const uint16_t LIMIT_SEQUENCE_COUNT = 16384;  // 2^14
static const uint16_t APID_IDLE_PACKET = 0x7FF;

constexpr uint16_t getSpacePacketIdFromApid(bool isTc, uint16_t apid,
                                            bool secondaryHeaderFlag = true) {
  return ((isTc << 4) | (secondaryHeaderFlag << 3) | ((apid >> 8) & 0x07)) << 8 | (apid & 0x00ff);
}

constexpr uint16_t getTcSpacePacketIdFromApid(uint16_t apid, bool secondaryHeaderFlag = true) {
  return getSpacePacketIdFromApid(true, apid, secondaryHeaderFlag);
}

constexpr uint16_t getTmSpacePacketIdFromApid(uint16_t apid, bool secondaryHeaderFlag = true) {
  return getSpacePacketIdFromApid(false, apid, secondaryHeaderFlag);
}

}  // namespace ccsds

class SpacePacketIF {
 public:
  /**
   * This definition defines the CRC size in byte.
   */
  static const uint8_t CRC_SIZE = 2;
  /**
   * This is the minimum size of a SpacePacket.
   */
  static const uint16_t MINIMUM_SIZE = sizeof(CCSDSPrimaryHeader) + CRC_SIZE;

  virtual ~SpacePacketIF() = default;

  /**
   * Returns the complete first two bytes of the packet, which together form
   * the CCSDS packet ID
   * @return	The CCSDS packet ID
   */
  [[nodiscard]] virtual uint16_t getPacketId() const = 0;
  /**
   * Returns the third and the fourth byte of the CCSDS header which are the packet sequence
   * control field
   * @return
   */
  [[nodiscard]] virtual uint16_t getPacketSeqCtrl() const = 0;
  /**
   * Returns the fifth and the sixth byte of the CCSDS header which is the packet length field
   * @return
   */
  [[nodiscard]] virtual uint16_t getPacketDataLen() const = 0;

  [[nodiscard]] virtual uint16_t getApid() const {
    uint16_t packetId = getPacketId();
    // Uppermost 11 bits of packet ID
    return ((packetId >> 8) & 0b111) | (packetId & 0xFF);
  }

  [[nodiscard]] virtual uint8_t getVersion() const {
    uint16_t packetId = getPacketId();
    return (packetId >> 13) & 0b111;
  }

  [[nodiscard]] virtual ccsds::PacketType getPacketType() const {
    return static_cast<ccsds::PacketType>((getPacketId() >> 12) & 0b1);
  }

  [[nodiscard]] virtual bool isTc() const { return getPacketType() == ccsds::PacketType::TC; }

  [[nodiscard]] virtual bool isTm() const { return not isTc(); }

  /**
   * The CCSDS header provides a secondary header flag (the fifth-highest bit),
   *  which is checked with this method.
   * @return	Returns true if the bit is set and false if not.
   */
  [[nodiscard]] virtual bool hasSecHeader() const { return (getPacketId() >> 11) & 0b1; }

  [[nodiscard]] virtual ccsds::SequenceFlags getSequenceFlags() const {
    return static_cast<ccsds::SequenceFlags>(getPacketSeqCtrl() >> 14 & 0b11);
  }

  [[nodiscard]] virtual uint16_t getSequenceCount() const { return getPacketSeqCtrl() & 0x3FFF; }
};

#endif  // FSFW_TMTCPACKET_SPACEPACKETIF_H
