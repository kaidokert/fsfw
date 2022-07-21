#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/tmtcpacket/pus/tc/PusTcCreator.h"
#include "fsfw/tmtcpacket/pus/tc/PusTcReader.h"

TEST_CASE("PUS TC Reader", "[pus-tc-reader]") {
  auto packetId = PacketId(ccsds::PacketType::TC, true, 0x02);
  auto spParams =
      SpacePacketParams(packetId, PacketSeqCtrl(ccsds::SequenceFlags::UNSEGMENTED, 0x34), 0x00);
  auto pusParams = PusTcParams(17, 1);
  PusTcCreator creator(spParams, pusParams);
  std::array<uint8_t, 32> buf{};
  uint8_t* dataPtr = buf.data();
  size_t serLen = 0;
  PusTcReader reader;
  auto checkReaderFields = [&](PusTcReader& reader) {
    REQUIRE(not reader.isNull());
    REQUIRE(reader.getPacketType() == ccsds::PacketType::TC);
    REQUIRE(reader.getApid() == 0x02);
    REQUIRE(reader.getService() == 17);
    REQUIRE(reader.getSubService() == 1);
    REQUIRE(reader.getFullPacketLen() == 13);
    REQUIRE(reader.getPacketDataLen() == 6);
    REQUIRE(reader.getPusVersion() == 2);
    REQUIRE(reader.getSequenceCount() == 0x34);
    REQUIRE(reader.getUserData() == nullptr);
    REQUIRE(reader.getUserDataLen() == 0);
    REQUIRE(reader.getFullData() == buf.data());
    REQUIRE(reader.getSourceId() == 0x00);
    REQUIRE(reader.getAcknowledgeFlags() == 0b1111);
    // This value was verified to be correct
    REQUIRE(reader.getErrorControl() == 0xee63);
  };

  SECTION("State") {
    REQUIRE(creator.serialize(&dataPtr, &serLen, buf.size()) == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(reader.isNull());

    SECTION("Setter") {
      REQUIRE(reader.setReadOnlyData(buf.data(), serLen) == HasReturnvaluesIF::RETURN_OK);
      REQUIRE(reader.parseDataWithCrcCheck() == HasReturnvaluesIF::RETURN_OK);
      checkReaderFields(reader);
    }
    SECTION("Directly Constructed") {
      PusTcReader secondReader(buf.data(), serLen);
      REQUIRE(not secondReader.isNull());
      REQUIRE(secondReader.parseDataWithCrcCheck() == HasReturnvaluesIF::RETURN_OK);
      checkReaderFields(secondReader);
    }
  }

  SECTION("Invalid CRC") {
    REQUIRE(creator.serialize(&dataPtr, &serLen, buf.size()) == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(reader.setReadOnlyData(buf.data(), serLen) == HasReturnvaluesIF::RETURN_OK);
    buf[11] = 0x00;
    REQUIRE(reader.parseDataWithCrcCheck() == PusIF::INVALID_CRC_16);
  }

  SECTION("Invalid CRC but no check") {
    REQUIRE(creator.serialize(&dataPtr, &serLen, buf.size()) == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(reader.setReadOnlyData(buf.data(), serLen) == HasReturnvaluesIF::RETURN_OK);
    buf[11] = 0x00;
    REQUIRE(reader.parseDataWithoutCrcCheck() == HasReturnvaluesIF::RETURN_OK);
  }

  SECTION("With application data") {
    auto& params = creator.getPusParams();
    std::array<uint8_t, 3> data{1, 2, 3};
    creator.setRawAppData({data.data(), data.size()});
    REQUIRE(creator.serialize(&dataPtr, &serLen, buf.size()) == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(reader.setReadOnlyData(buf.data(), serLen) == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(reader.parseDataWithCrcCheck() == HasReturnvaluesIF::RETURN_OK);
    const uint8_t* userDataPtr = reader.getUserData();
    REQUIRE(userDataPtr != nullptr);
    REQUIRE(reader.getUserDataLen() == 3);
    REQUIRE(userDataPtr[0] == 1);
    REQUIRE(userDataPtr[1] == 2);
    REQUIRE(userDataPtr[2] == 3);
  }
}