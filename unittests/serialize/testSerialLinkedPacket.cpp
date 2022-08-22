#include <fsfw/globalfunctions/arrayprinter.h>

#include <array>
#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"
#include "SerialLinkedListAdapterPacket.h"

TEST_CASE("Serial Linked Packet", "[SerLinkPacket]") {
  // perform set-up here
  uint32_t header = 42;
  std::array<uint8_t, 3> testArray{1, 2, 3};
  uint32_t tail = 96;
  size_t packetMaxSize = 256;
  uint8_t packet[packetMaxSize] = {};
  size_t packetLen = 0;

  SECTION("Test Deserialization with Serial Buffer Adapter.") {
    // This is a serialization of a packet, made "manually".
    // We generate a packet which store data big-endian by swapping some
    // values. (like coming from ground).
    header = EndianConverter::convertBigEndian(header);
    std::memcpy(packet, &header, sizeof(header));
    packetLen += sizeof(header);

    std::copy(testArray.data(), testArray.data() + testArray.size(), packet + packetLen);
    packetLen += testArray.size();

    tail = EndianConverter::convertBigEndian(tail);
    std::memcpy(packet + packetLen, &tail, sizeof(tail));
    packetLen += sizeof(tail);

    // arrayprinter::print(packet, packetLen, OutputType::DEC);

    // This is the buffer which will be filled when testClass.deSerialize
    // is called.
    std::array<uint8_t, 3> bufferAdaptee = {};
    TestPacket testClass(packet, packetLen, bufferAdaptee.data(), bufferAdaptee.size());
    const uint8_t* readOnlyPointer = packet;
    // Deserialize big endian packet by setting bigEndian to true.
    ReturnValue_t result =
        testClass.deSerialize(&readOnlyPointer, &packetLen, SerializeIF::Endianness::BIG);
    REQUIRE(result == returnvalue::OK);
    CHECK(testClass.getHeader() == 42);
    // Equivalent check.
    // CHECK(testClass.getBuffer()[0] == 1);
    CHECK(bufferAdaptee[0] == 1);
    CHECK(bufferAdaptee[1] == 2);
    CHECK(bufferAdaptee[2] == 3);
    CHECK(testClass.getTail() == 96);
  }

  SECTION("Test Serialization") {
    // Same process as performed in setup, this time using the class
    // instead of doing it manually.
    TestPacket testClass(header, tail, testArray.data(), testArray.size());
    size_t serializedSize = 0;
    uint8_t* packetPointer = packet;
    // serialize for ground: bigEndian = true.
    ReturnValue_t result = testClass.serialize(&packetPointer, &serializedSize, packetMaxSize,
                                               SerializeIF::Endianness::BIG);
    REQUIRE(result == returnvalue::OK);
    // Result should be big endian now.
    CHECK(packet[3] == 42);
    CHECK(packet[4] == 1);
    CHECK(packet[5] == 2);
    CHECK(packet[6] == 3);
    CHECK(packet[10] == 96);
  }

  // perform tear-down here
}
