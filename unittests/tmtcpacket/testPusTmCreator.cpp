#include <catch2/catch_test_macros.hpp>

#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/tmtcpacket/pus/tm.h"
#include "fsfw/globalfunctions/CRC.h"
#include "mocks/CdsShortTimestamperMock.h"

TEST_CASE("PUS TM Creator", "[pus-tm-creator]") {
  auto packetId = PacketId(ccsds::PacketType::TC, true, 0xef);
  auto spParams =
      SpacePacketParams(packetId, PacketSeqCtrl(ccsds::SequenceFlags::UNSEGMENTED, 22), 0x00);
  auto timeStamper = CdsShortTimestamperMock();
  auto pusTmParams = PusTmParams(17, 2, &timeStamper);
  timeStamper.valueToStamp = {1, 2, 3, 4, 5, 6, 7};
  PusTmCreator creator(spParams, pusTmParams);
  std::array<uint8_t, 32> buf{};
  uint8_t* dataPtr = buf.data();
  size_t serLen = 0;

  SECTION("State") {
    REQUIRE(creator.isTm());
    REQUIRE(creator.getApid() == 0xef);
    REQUIRE(creator.getService() == 17);
    REQUIRE(creator.getSubService() == 2);
    REQUIRE(creator.getTimestamper() == &timeStamper);
    REQUIRE(creator.getSequenceFlags() == ccsds::SequenceFlags::UNSEGMENTED);
    REQUIRE(creator.getSequenceCount() == 22);
    // 6 bytes CCSDS header, 7 bytes secondary header, 7 bytes CDS short timestamp,
    // 0 bytes application data, 2 bytes CRC
    REQUIRE(creator.getFullPacketLen() == 22);
    // As specified in standard, the data length fields is the total size of the packet without
    // the primary header minus 1
    REQUIRE(creator.getPacketDataLen() == 15);
    REQUIRE(timeStamper.getSizeCallCount == 1);
  }

  SECTION("Serialization") {
    REQUIRE(creator.serialize(&dataPtr, &serLen, buf.size()) == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(buf[0] == 0x08);
    REQUIRE(buf[1] == 0xef);
    // Unsegmented is the default
    REQUIRE(buf[2] == 0xc0);
    REQUIRE(buf[3] == 22);
    REQUIRE(buf[4] == 0);
    REQUIRE(buf[5] == 15);
    REQUIRE(((buf[6] >> 4) & 0b1111) == ecss::PusVersion::PUS_C);
    // SC time reference field
    REQUIRE((buf[6] & 0b1111) == 0);
    // Service and subservice field
    REQUIRE(buf[7] == 17);
    REQUIRE(buf[8] == 2);
    // Message Sequence Count
    REQUIRE(((buf[9] << 8) | buf[10]) == 0);
    // Destination ID
    REQUIRE(((buf[11] << 8) | buf[12]) == 0);
    // Custom timestamp
    for(size_t i = 1; i < 8; i++) {
      REQUIRE(buf[12 + i] == i);
    }
    REQUIRE(serLen == 22);
    REQUIRE(CRC::crc16ccitt(buf.data(), serLen) == 0);
    REQUIRE(buf[20] == 0x03);
    REQUIRE(buf[21] == 0x79);
  }
}