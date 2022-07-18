#ifndef FSFW_TMTCPACKET_SPACEPACKETCREATOR_H
#define FSFW_TMTCPACKET_SPACEPACKETCREATOR_H

#include "SpacePacketIF.h"
#include "fsfw/serialize/SerializeIF.h"

class SpacePacketCreator : public SerializeIF, public SpacePacketIF {
 public:
  SpacePacketCreator(ccsds::PacketType packetType, bool secHeaderFlag, uint16_t apid,
                     ccsds::SequenceFlags seqFlags, uint16_t seqCount, uint16_t dataLen,
                     uint8_t version = 0);
  SpacePacketCreator(uint16_t packetId, uint16_t packetSeqCtrl, uint16_t dataLen,
                     uint8_t version = 0);
  [[nodiscard]] bool isValid() const;
  [[nodiscard]] uint16_t getPacketId() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrl() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;
  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;
  [[nodiscard]] size_t getSerializedSize() const override;
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;

 private:
  bool valid;
  uint16_t packetId;
  uint16_t packetSeqCtrl;
  uint16_t dataLen;
  uint8_t version;
};
#endif  // FSFW_TMTCPACKET_SPACEPACKETCREATOR_H
