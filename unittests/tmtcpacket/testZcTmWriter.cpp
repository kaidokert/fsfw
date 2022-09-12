#include <catch2/catch_test_macros.hpp>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/tmtcpacket/pus/tm.h"
#include "fsfw/tmtcpacket/pus/tm/PusTmZcWriter.h"
#include "mocks/CdsShortTimestamperMock.h"

TEST_CASE("TM ZC Helper", "[tm-zc-helper]") {
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

  SECTION("No Crash For Uninitialized Object") {
    REQUIRE(creator.serializeBe(dataPtr, serLen, buf.size()) == returnvalue::OK);
    PusTmZeroCopyWriter writer(timeStamper, dataPtr, serLen);
    REQUIRE(writer.getSequenceCount() == 22);
    writer.setSequenceCount(23);
    // Can't set anything, parse function was not called
    REQUIRE(writer.getSequenceCount() == 22);
    writer.updateErrorControl();
  }

  SECTION("Basic") {
    REQUIRE(creator.serializeBe(dataPtr, serLen, buf.size()) == returnvalue::OK);
    PusTmZeroCopyWriter writer(timeStamper, dataPtr, serLen);
    REQUIRE(writer.parseDataWithoutCrcCheck() == returnvalue::OK);
    REQUIRE(writer.getSequenceCount() == 22);
    writer.setSequenceCount(23);
    REQUIRE(writer.getSequenceCount() == 23);
    // CRC is invalid now
    REQUIRE(CRC::crc16ccitt(dataPtr, serLen) != 0);
    writer.updateErrorControl();
    REQUIRE(CRC::crc16ccitt(dataPtr, serLen) == 0);
  }
}