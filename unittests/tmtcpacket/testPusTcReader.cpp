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

  SECTION("State") {
    REQUIRE(creator.serializeBe(&dataPtr, &serLen, buf.size()) == returnvalue::OK);
    REQUIRE(reader.isNull());
    REQUIRE(not reader);
    PusTcReader* readerPtr = nullptr;
    bool callDelete = false;
    SECTION("Setter") {
      readerPtr = &reader;
      REQUIRE(readerPtr->setReadOnlyData(buf.data(), serLen) == returnvalue::OK);
      REQUIRE(readerPtr->parseDataWithCrcCheck() == returnvalue::OK);
    }
    SECTION("Directly Constructed") {
      callDelete = true;
      readerPtr = new PusTcReader(buf.data(), serLen);
      REQUIRE(not readerPtr->isNull());
      REQUIRE(readerPtr->parseDataWithCrcCheck() == returnvalue::OK);
    }
    REQUIRE(not readerPtr->isNull());
    REQUIRE(*readerPtr);
    REQUIRE(readerPtr->getPacketType() == ccsds::PacketType::TC);
    REQUIRE(readerPtr->getApid() == 0x02);
    REQUIRE(readerPtr->getService() == 17);
    REQUIRE(readerPtr->getSubService() == 1);
    REQUIRE(readerPtr->getFullPacketLen() == 13);
    REQUIRE(readerPtr->getPacketDataLen() == 6);
    REQUIRE(readerPtr->getPusVersion() == 2);
    REQUIRE(readerPtr->getSequenceCount() == 0x34);
    REQUIRE(readerPtr->getUserData() == nullptr);
    REQUIRE(readerPtr->getUserDataLen() == 0);
    REQUIRE(readerPtr->getFullData() == buf.data());
    REQUIRE(readerPtr->getSourceId() == 0x00);
    REQUIRE(readerPtr->getAcknowledgeFlags() == 0b1111);
    // This value was verified to be correct
    REQUIRE(readerPtr->getErrorControl() == 0xee63);
    if (callDelete) {
      delete readerPtr;
    }
  }

  SECTION("Invalid CRC") {
    REQUIRE(creator.serializeBe(&dataPtr, &serLen, buf.size()) == returnvalue::OK);
    REQUIRE(reader.setReadOnlyData(buf.data(), serLen) == returnvalue::OK);
    buf[11] = 0x00;
    REQUIRE(reader.parseDataWithCrcCheck() == PusIF::INVALID_CRC_16);
  }

  SECTION("Invalid CRC but no check") {
    REQUIRE(creator.serializeBe(&dataPtr, &serLen, buf.size()) == returnvalue::OK);
    REQUIRE(reader.setReadOnlyData(buf.data(), serLen) == returnvalue::OK);
    buf[11] = 0x00;
    REQUIRE(reader.parseDataWithoutCrcCheck() == returnvalue::OK);
  }

  SECTION("With application data") {
    auto& params = creator.getPusParams();
    std::array<uint8_t, 3> data{1, 2, 3};
    creator.setRawUserData(data.data(), data.size());
    REQUIRE(creator.serializeBe(&dataPtr, &serLen, buf.size()) == returnvalue::OK);
    REQUIRE(reader.setReadOnlyData(buf.data(), serLen) == returnvalue::OK);
    REQUIRE(reader.parseDataWithCrcCheck() == returnvalue::OK);
    const uint8_t* userDataPtr = reader.getUserData();
    REQUIRE(userDataPtr != nullptr);
    REQUIRE(reader.getUserDataLen() == 3);
    REQUIRE(userDataPtr[0] == 1);
    REQUIRE(userDataPtr[1] == 2);
    REQUIRE(userDataPtr[2] == 3);
  }
}