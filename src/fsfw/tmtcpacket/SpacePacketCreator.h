#ifndef FSFW_TMTCPACKET_SPACEPACKETCREATOR_H
#define FSFW_TMTCPACKET_SPACEPACKETCREATOR_H

#include "SpacePacketIF.h"
#include "fsfw/serialize/SerializeIF.h"

struct PacketId {
 public:
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

  ccsds::PacketType packetType;
  bool secHeaderFlag;
  uint16_t apid;
};

struct PacketSeqCtrl {
 public:
  PacketSeqCtrl(ccsds::SequenceFlags seqFlags, uint16_t seqCount)
      : seqFlags(seqFlags), seqCount(seqCount) {}

  /**
   * NOTE: If the sequence control has an invalid value, the invalid bits will be cut off
   * @return
   */
  [[nodiscard]] uint16_t raw() const {
    return (static_cast<uint16_t>(seqFlags) << 14) | (seqCount & 0x3FFF);
  }
  ccsds::SequenceFlags seqFlags;
  uint16_t seqCount;
};

class SpacePacketCreator : public SerializeIF, public SpacePacketIF {
 public:
  SpacePacketCreator(ccsds::PacketType packetType, bool secHeaderFlag, uint16_t apid,
                     ccsds::SequenceFlags seqFlags, uint16_t seqCount, uint16_t dataLen,
                     uint8_t version = 0);
  SpacePacketCreator(PacketId packetId, PacketSeqCtrl psc, uint16_t dataLen, uint8_t version = 0);
  [[nodiscard]] bool isValid() const;
  [[nodiscard]] uint16_t getPacketId() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrl() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;

  void setApid(uint16_t apid);
  void setSeqCount(uint16_t seqCount);
  void setSeqFlags(ccsds::SequenceFlags flags);
  void setDataLen(uint16_t dataLen);

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize) const;
  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;

 private:
  void checkFieldValidity();

  bool valid{};
  PacketId packetId;
  PacketSeqCtrl packetSeqCtrl;
  uint16_t dataLen;
  uint8_t version;
};
#endif  // FSFW_TMTCPACKET_SPACEPACKETCREATOR_H
