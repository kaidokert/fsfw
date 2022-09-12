#ifndef FSFW_TMTCPACKET_SPACEPACKETIF_H
#define FSFW_TMTCPACKET_SPACEPACKETIF_H

#include <cstddef>
#include <cstdint>

#include "PacketId.h"
#include "PacketSeqCtrl.h"
#include "fsfw/tmtcpacket/ccsds/defs.h"
#include "header.h"

class SpacePacketIF {
 public:
  /**
   * This definition defines the CRC size in byte.
   */
  static const uint8_t CRC_SIZE = 2;
  /**
   * This is the minimum size of a SpacePacket.
   */
  static const uint16_t MIN_CCSDS_SIZE = sizeof(ccsds::PrimaryHeader) + CRC_SIZE;

  virtual ~SpacePacketIF() = default;

  /**
   * CCSDS header always has 6 bytes
   * @return
   */
  static constexpr size_t getHeaderLen() { return ccsds::HEADER_LEN; }

  /**
   * Returns the complete first two bytes of the packet, which together form
   * the CCSDS packet ID
   * @return	The CCSDS packet ID
   */
  [[nodiscard]] virtual uint16_t getPacketIdRaw() const = 0;
  /**
   * Returns the third and the fourth byte of the CCSDS header which are the packet sequence
   * control field
   * @return
   */
  [[nodiscard]] virtual uint16_t getPacketSeqCtrlRaw() const = 0;
  /**
   * Returns the fifth and the sixth byte of the CCSDS header which is the packet length field
   * @return
   */
  [[nodiscard]] virtual uint16_t getPacketDataLen() const = 0;

  virtual PacketId getPacketId() { return PacketId::fromRaw(getPacketIdRaw()); }

  virtual PacketSeqCtrl getPacketSeqCtrl() { return PacketSeqCtrl::fromRaw(getPacketSeqCtrlRaw()); }

  [[nodiscard]] virtual uint16_t getApid() const {
    // Uppermost 11 bits of packet ID
    return getPacketIdRaw() & 0x7ff;
  }

  /**
   * Returns the CCSDS version number
   * @return
   */
  [[nodiscard]] virtual uint8_t getVersion() const {
    uint16_t packetId = getPacketIdRaw();
    return (packetId >> 13) & 0b111;
  }

  [[nodiscard]] virtual ccsds::PacketType getPacketType() const {
    return static_cast<ccsds::PacketType>((getPacketIdRaw() >> 12) & 0b1);
  }

  [[nodiscard]] virtual bool isTc() const { return getPacketType() == ccsds::PacketType::TC; }

  [[nodiscard]] virtual bool isTm() const { return not isTc(); }

  /**
   * The CCSDS header provides a secondary header flag (the fifth-highest bit),
   *  which is checked with this method.
   * @return	Returns true if the bit is set and false if not.
   */
  [[nodiscard]] virtual bool hasSecHeader() const { return (getPacketIdRaw() >> 11) & 0b1; }

  [[nodiscard]] virtual ccsds::SequenceFlags getSequenceFlags() const {
    return static_cast<ccsds::SequenceFlags>(getPacketSeqCtrlRaw() >> 14 & 0b11);
  }

  [[nodiscard]] virtual uint16_t getSequenceCount() const { return getPacketSeqCtrlRaw() & 0x3FFF; }

  /**
   * Returns the full packet length based of the packet data length field
   * @return
   */
  [[nodiscard]] virtual size_t getFullPacketLen() const {
    return ccsds::HEADER_LEN + getPacketDataLen() + 1;
  }
};

#endif  // FSFW_TMTCPACKET_SPACEPACKETIF_H
