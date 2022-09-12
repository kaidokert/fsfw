#ifndef FSFW_TMTCPACKET_SPACEPACKETCREATOR_H
#define FSFW_TMTCPACKET_SPACEPACKETCREATOR_H

#include <utility>

#include "PacketId.h"
#include "PacketSeqCtrl.h"
#include "SpacePacketIF.h"
#include "fsfw/serialize/SerializeIF.h"

struct SpacePacketParams {
  SpacePacketParams() = default;

  SpacePacketParams(PacketId packetId, PacketSeqCtrl psc, uint16_t dataLen)
      : packetId(std::move(packetId)), packetSeqCtrl(std::move(psc)), dataLen(dataLen) {}

  PacketId packetId{};
  PacketSeqCtrl packetSeqCtrl{};
  uint16_t dataLen = 0;
  uint8_t version = 0b000;
};

class SpacePacketCreator : public SpacePacketIF, public SerializeIF {
 public:
  SpacePacketCreator() = default;
  explicit SpacePacketCreator(SpacePacketParams params);

  bool operator==(const SpacePacketCreator &other) const;
  explicit operator bool() const;

  SpacePacketCreator(ccsds::PacketType packetType, bool secHeaderFlag, uint16_t apid,
                     ccsds::SequenceFlags seqFlags, uint16_t seqCount, uint16_t dataLen,
                     uint8_t version = 0);

  [[nodiscard]] bool isValid() const;
  [[nodiscard]] uint16_t getPacketIdRaw() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrlRaw() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;

  SpacePacketParams &getParams();
  void setParams(SpacePacketParams params);
  void setSecHeaderFlag();
  void setPacketType(ccsds::PacketType type);
  void setApid(uint16_t apid);
  void setSeqCount(uint16_t seqCount);
  void setSeqFlags(ccsds::SequenceFlags flags);
  void setDataLen(uint16_t dataLen);

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;

 private:
  void checkFieldValidity();
  bool valid{};
  SpacePacketParams params{};
};
#endif  // FSFW_TMTCPACKET_SPACEPACKETCREATOR_H
