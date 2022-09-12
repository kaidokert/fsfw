#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/tmtcpacket/pus/tc.h"
#include "mocks/SimpleSerializable.h"

TEST_CASE("PUS TC Creator", "[pus-tc-creator]") {
  auto packetId = PacketId(ccsds::PacketType::TC, true, 0x02);
  auto spParams =
      SpacePacketParams(packetId, PacketSeqCtrl(ccsds::SequenceFlags::UNSEGMENTED, 0x34), 0x00);
  auto pusParams = PusTcParams(17, 1);
  PusTcCreator creator(spParams, pusParams);
  std::array<uint8_t, 32> buf{};
  uint8_t* dataPtr = buf.data();
  size_t serLen = 0;

  SECTION("State") {
    CHECK(creator.isTc());
    CHECK(creator.hasSecHeader());
    CHECK(creator.getService() == 17);
    CHECK(creator.getSubService() == 1);
    CHECK(creator.getApid() == 0x02);
    CHECK(creator.getPusVersion() == 2);
    CHECK(creator.getAcknowledgeFlags() == 0b1111);
    CHECK(creator.getSourceId() == 0x00);
    CHECK(creator.getPacketSeqCtrlRaw() == 0xc034);
    // bytes CCSDS header, 5 bytes secondary header, 2 bytes CRC, 3 bytes app data
    CHECK(creator.getFullPacketLen() == 13);
    // The data length field is the full packet length minus the primary header minus 1
    CHECK(creator.getPacketDataLen() == 6);
    auto& paramsLocal = creator.getSpParams();
    CHECK(paramsLocal.packetId == packetId);
  }

  SECTION("Serialized") {
    REQUIRE(creator.serializeBe(&dataPtr, &serLen, buf.size()) == returnvalue::OK);
    REQUIRE(serLen == 13);
    REQUIRE(buf[0] == 0x18);
    REQUIRE(buf[1] == 0x02);
    // Unsegmented, first 2 bits 11
    REQUIRE(buf[2] == 0xc0);
    // Packet Sequence count only occupies lower byte of packet sequence control
    REQUIRE(buf[3] == 0x34);
    // Data length packed big endian
    REQUIRE(buf[4] == 0x00);
    REQUIRE(buf[5] == 0x06);
    // PUS Version C (2)
    REQUIRE(((buf[6] >> 4) & 0b1111) == 2);
    // All Ack Fields is default
    REQUIRE((buf[6] & 0b1111) == 0b1111);
    // Service and subservice
    REQUIRE(buf[7] == 17);
    REQUIRE(buf[8] == 1);
    // Source ID is 0
    REQUIRE(((buf[9] << 8) | buf[10]) == 0);
    // CRC16 check
    REQUIRE(CRC::crc16ccitt(buf.data(), serLen) == 0);
    REQUIRE(buf[11] == 0xee);
    REQUIRE(buf[12] == 0x63);
  }

  SECTION("Custom Source ID") {
    auto& params = creator.getPusParams();
    params.sourceId = 0x5ff;
    REQUIRE(creator.getSourceId() == 0x5ff);
    REQUIRE(creator.serializeBe(&dataPtr, &serLen, buf.size()) == returnvalue::OK);
    REQUIRE(((buf[9] << 8) | buf[10]) == 0x5ff);
  }

  SECTION("Test with Application Data Raw") {
    auto& params = creator.getPusParams();
    std::array<uint8_t, 3> data{1, 2, 3};
    params.setRawAppData(data.data(), data.size());
    // To get correct size information, the SP length field needs to be updated automatically
    REQUIRE(creator.getSerializedSize() == 13);
    creator.updateSpLengthField();
    REQUIRE(creator.getSerializedSize() == 16);
    REQUIRE(creator.serializeBe(&dataPtr, &serLen, buf.size()) == returnvalue::OK);
    REQUIRE(serLen == 16);
    REQUIRE(buf[11] == 1);
    REQUIRE(buf[12] == 2);
    REQUIRE(buf[13] == 3);
  }

  SECTION("Test with Application Data Serializable") {
    auto& params = creator.getPusParams();
    auto simpleSer = SimpleSerializable();
    creator.setSerializableUserData(simpleSer);
    REQUIRE(creator.getSerializedSize() == 16);
    REQUIRE(creator.serializeBe(&dataPtr, &serLen, buf.size()) == returnvalue::OK);
    REQUIRE(serLen == 16);
    REQUIRE(buf[11] == 1);
    REQUIRE(buf[12] == 2);
    REQUIRE(buf[13] == 3);
  }

  SECTION("Test with Application Data Serializable Simple Ser API") {
    auto& params = creator.getPusParams();
    auto simpleSer = SimpleSerializable();
    creator.setSerializableUserData(simpleSer);
    REQUIRE(creator.getSerializedSize() == 16);
    REQUIRE(creator.serializeBe(dataPtr, serLen, buf.size()) == returnvalue::OK);
    REQUIRE(serLen == 16);
    REQUIRE(buf[11] == 1);
    REQUIRE(buf[12] == 2);
    REQUIRE(buf[13] == 3);
  }

  SECTION("Deserialization Fails") {
    SerializeIF& deser = creator;
    size_t deserLen = buf.size();
    const uint8_t* roPtr = buf.data();
    REQUIRE(deser.deSerialize(&roPtr, &deserLen, SerializeIF::Endianness::NETWORK) ==
            returnvalue::FAILED);
  }

  SECTION("Serialize with invalid buffer length") {
    size_t reqSize = creator.getSerializedSize();
    for (size_t maxSize = 0; maxSize < reqSize; maxSize++) {
      dataPtr = buf.data();
      serLen = 0;
      REQUIRE(creator.serializeBe(&dataPtr, &serLen, maxSize) == SerializeIF::BUFFER_TOO_SHORT);
    }
  }

  SECTION("Invalid PUS Version") {
    auto& params = creator.getPusParams();
    params.pusVersion = 0;
    REQUIRE(creator.serializeBe(&dataPtr, &serLen, buf.size()) == PusIF::INVALID_PUS_VERSION);
  }
}