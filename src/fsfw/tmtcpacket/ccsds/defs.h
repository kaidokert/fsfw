#ifndef CCSDS_HEADER_H_
#define CCSDS_HEADER_H_

#include <cstdint>

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

constexpr uint16_t getTcSpacePacketIdFromApid(uint16_t apid, bool secondaryHeaderFlag) {
  return getSpacePacketIdFromApid(true, apid, secondaryHeaderFlag);
}

constexpr uint16_t getTmSpacePacketIdFromApid(uint16_t apid, bool secondaryHeaderFlag) {
  return getSpacePacketIdFromApid(false, apid, secondaryHeaderFlag);
}

}  // namespace ccsds

#endif /* CCSDS_HEADER_H_ */
