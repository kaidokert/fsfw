#include <catch2/catch_test_macros.hpp>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/tmtcpacket/pus/tm.h"
#include "mocks/CdsShortTimestamperMock.h"
#include "mocks/SimpleSerializable.h"

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
    CHECK(creator.isTm());
    CHECK(creator.hasSecHeader());
    CHECK(creator.getApid() == 0xef);
    CHECK(creator.getPusVersion() == 2);
    CHECK(creator.getScTimeRefStatus() == 0);
    CHECK(creator.getService() == 17);
    CHECK(creator.getSubService() == 2);
    CHECK(creator.getTimestamper() == &timeStamper);
    CHECK(creator.getSequenceFlags() == ccsds::SequenceFlags::UNSEGMENTED);
    CHECK(creator.getSequenceCount() == 22);
    // 6 bytes CCSDS header, 7 bytes secondary header, 7 bytes CDS short timestamp,
    // 0 bytes application data, 2 bytes CRC
    CHECK(creator.getFullPacketLen() == 22);
    // As specified in standard, the data length fields is the total size of the packet without
    // the primary header minus 1
    CHECK(creator.getPacketDataLen() == 15);
    CHECK(timeStamper.getSizeCallCount == 1);
  }

  SECTION("SP Params") {
    auto& spParamsRef = creator.getSpParams();
    REQUIRE(spParamsRef.dataLen == 15);
    REQUIRE(spParamsRef.packetId.apid == 0xef);
  }

  SECTION("Serialization") {
    REQUIRE(creator.SerializeIF::serializeBe(&dataPtr, &serLen, buf.size()) == returnvalue::OK);
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
    for (size_t i = 1; i < 8; i++) {
      REQUIRE(buf[12 + i] == i);
    }
    REQUIRE(serLen == 22);
    REQUIRE(CRC::crc16ccitt(buf.data(), serLen) == 0);
    REQUIRE(buf[20] == 0x03);
    REQUIRE(buf[21] == 0x79);
    REQUIRE(timeStamper.serializeCallCount == 1);
  }

  SECTION("Custom Fields") {
    creator.setApid(0x3ff);
    SECTION("Using Params") {
      auto& pusParams = creator.getParams();
      pusParams.secHeader.destId = 0xfff;
      pusParams.secHeader.messageTypeCounter = 0x313;
    }
    SECTION("Using Setters") {
      auto& pusParams = creator.getParams();
      creator.setDestId(0xfff);
      creator.setMessageTypeCounter(0x313);
    }
    REQUIRE(creator.getApid() == 0x3ff);
    REQUIRE(creator.getDestId() == 0xfff);
    REQUIRE(creator.getMessageTypeCounter() == 0x313);
    REQUIRE(creator.serializeBe(&dataPtr, &serLen, buf.size()) == returnvalue::OK);
    // Message Sequence Count
    REQUIRE(((buf[9] << 8) | buf[10]) == 0x313);
    // Destination ID
    REQUIRE(((buf[11] << 8) | buf[12]) == 0xfff);
  }

  SECTION("Deserialization fails") {
    SerializeIF& deser = creator;
    const uint8_t* roDataPtr = nullptr;
    REQUIRE(deser.deSerialize(&roDataPtr, &serLen, SerializeIF::Endianness::NETWORK) ==
            returnvalue::FAILED);
  }

  SECTION("Serialize with Raw Data") {
    std::array<uint8_t, 3> data{1, 2, 3};
    creator.setRawUserData(data.data(), data.size());
    REQUIRE(creator.getFullPacketLen() == 25);
    REQUIRE(creator.serializeBe(&dataPtr, &serLen, buf.size()) == returnvalue::OK);
    REQUIRE(buf[20] == 1);
    REQUIRE(buf[21] == 2);
    REQUIRE(buf[22] == 3);
  }

  SECTION("Serialize with Serializable") {
    auto simpleSer = SimpleSerializable();
    creator.setSerializableUserData(simpleSer);
    REQUIRE(creator.getFullPacketLen() == 25);
    REQUIRE(creator.serialize(&dataPtr, &serLen, buf.size(), SerializeIF::Endianness::NETWORK) ==
            returnvalue::OK);
    REQUIRE(buf[20] == 1);
    REQUIRE(buf[21] == 2);
    REQUIRE(buf[22] == 3);
  }

  SECTION("Empty Ctor") {
    PusTmCreator creatorFromEmptyCtor;
    // 6 bytes CCSDS header, 7 bytes secondary header, no timestamp (IF is null),
    // 0 bytes application data, 2 bytes CRC
    REQUIRE(creatorFromEmptyCtor.getFullPacketLen() == 15);
    // As specified in standard, the data length fields is the total size of the packet without
    // the primary header minus 1
    REQUIRE(creatorFromEmptyCtor.getPacketDataLen() == 8);
    creatorFromEmptyCtor.setTimeStamper(timeStamper);
    REQUIRE(creatorFromEmptyCtor.getFullPacketLen() == 22);
    REQUIRE(creatorFromEmptyCtor.getPacketDataLen() == 15);
  }

  SECTION("Invalid Buffer Sizes") {
    size_t reqSize = creator.getSerializedSize();
    for (size_t maxSize = 0; maxSize < reqSize; maxSize++) {
      dataPtr = buf.data();
      serLen = 0;
      REQUIRE(creator.serialize(&dataPtr, &serLen, maxSize, SerializeIF::Endianness::NETWORK) ==
              SerializeIF::BUFFER_TOO_SHORT);
    }
  }

  SECTION("No CRC Generation") {
    creator.disableCrcCalculation();
    REQUIRE(not creator.crcCalculationEnabled());
    REQUIRE(creator.serializeBe(dataPtr, serLen, buf.size()) == returnvalue::OK);
    REQUIRE(serLen == 22);
    REQUIRE(buf[20] == 0x00);
    REQUIRE(buf[21] == 0x00);
  }
}