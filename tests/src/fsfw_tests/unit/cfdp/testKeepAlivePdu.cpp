#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/pdu/KeepAlivePduDeserializer.h"
#include "fsfw/cfdp/pdu/KeepAlivePduSerializer.h"
#include "fsfw/globalfunctions/arrayprinter.h"

TEST_CASE("Keep Alive PDU", "[KeepAlivePdu]") {
  using namespace cfdp;
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  std::array<uint8_t, 256> kaBuffer = {};
  uint8_t* buffer = kaBuffer.data();
  size_t sz = 0;
  EntityId destId(WidthInBytes::TWO_BYTES, 2);
  TransactionSeqNum seqNum(WidthInBytes::TWO_BYTES, 15);
  EntityId sourceId(WidthInBytes::TWO_BYTES, 1);
  PduConfig pduConf(TransmissionModes::ACKNOWLEDGED, seqNum, sourceId, destId);

  FileSize progress(0x50);

  SECTION("Serialize") {
    KeepAlivePduSerializer serializer(pduConf, progress);
    result = serializer.serialize(&buffer, &sz, kaBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(kaBuffer[10] == cfdp::FileDirectives::KEEP_ALIVE);
    uint32_t fsRaw = 0;
    result = SerializeAdapter::deSerialize(&fsRaw, kaBuffer.data() + 11, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(fsRaw == 0x50);
    REQUIRE(sz == 15);
    REQUIRE(serializer.getWholePduSize() == 15);
    REQUIRE(serializer.getPduDataFieldLen() == 5);

    pduConf.largeFile = true;
    serializer.updateDirectiveFieldLen();
    buffer = kaBuffer.data();
    sz = 0;
    result = serializer.serialize(&buffer, &sz, kaBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(serializer.getWholePduSize() == 19);
    REQUIRE(serializer.getPduDataFieldLen() == 9);
    uint64_t fsRawLarge = 0;
    result = SerializeAdapter::deSerialize(&fsRawLarge, kaBuffer.data() + 11, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(fsRawLarge == 0x50);

    for (size_t invalidMaxSz = 0; invalidMaxSz < sz; invalidMaxSz++) {
      buffer = kaBuffer.data();
      sz = 0;
      result = serializer.serialize(&buffer, &sz, invalidMaxSz, SerializeIF::Endianness::NETWORK);
      REQUIRE(result != HasReturnvaluesIF::RETURN_OK);
    }
  }

  SECTION("Deserialize") {
    KeepAlivePduSerializer serializer(pduConf, progress);
    result = serializer.serialize(&buffer, &sz, kaBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

    // Set another file size
    progress.setFileSize(200, false);
    KeepAlivePduDeserializer deserializer(kaBuffer.data(), kaBuffer.size(), progress);
    result = deserializer.parseData();
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    auto& progRef = deserializer.getProgress();
    // Should have been overwritten
    REQUIRE(progRef.getSize() == 0x50);
    sz = deserializer.getWholePduSize();

    // invalid max size
    for (size_t invalidMaxSz = 0; invalidMaxSz < sz; invalidMaxSz++) {
      deserializer.setData(kaBuffer.data(), invalidMaxSz);
      result = deserializer.parseData();
      REQUIRE(result != HasReturnvaluesIF::RETURN_OK);
    }
  }
}
