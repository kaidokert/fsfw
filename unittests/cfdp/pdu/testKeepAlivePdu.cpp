#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/pdu/KeepAlivePduCreator.h"
#include "fsfw/cfdp/pdu/KeepAlivePduReader.h"
#include "fsfw/globalfunctions/arrayprinter.h"

TEST_CASE("Keep Alive PDU", "[cfdp][pdu]") {
  using namespace cfdp;
  ReturnValue_t result = returnvalue::OK;
  std::array<uint8_t, 256> kaBuffer = {};
  uint8_t* buffer = kaBuffer.data();
  size_t sz = 0;
  EntityId destId(WidthInBytes::TWO_BYTES, 2);
  TransactionSeqNum seqNum(WidthInBytes::TWO_BYTES, 15);
  EntityId sourceId(WidthInBytes::TWO_BYTES, 1);
  PduConfig pduConf(sourceId, destId, TransmissionMode::ACKNOWLEDGED, seqNum);

  FileSize progress(0x50);

  SECTION("Serialize") {
    KeepAlivePduCreator serializer(pduConf, progress);
    result = serializer.serialize(&buffer, &sz, kaBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(kaBuffer[10] == cfdp::FileDirective::KEEP_ALIVE);
    uint32_t fsRaw = 0;
    result = SerializeAdapter::deSerialize(&fsRaw, kaBuffer.data() + 11, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(fsRaw == 0x50);
    REQUIRE(sz == 15);
    REQUIRE(serializer.getWholePduSize() == 15);
    REQUIRE(serializer.getPduDataFieldLen() == 5);

    pduConf.largeFile = true;
    serializer.updateDirectiveFieldLen();
    buffer = kaBuffer.data();
    sz = 0;
    result = serializer.serialize(&buffer, &sz, kaBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(serializer.getWholePduSize() == 19);
    REQUIRE(serializer.getPduDataFieldLen() == 9);
    uint64_t fsRawLarge = 0;
    result = SerializeAdapter::deSerialize(&fsRawLarge, kaBuffer.data() + 11, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(fsRawLarge == 0x50);

    for (size_t invalidMaxSz = 0; invalidMaxSz < sz; invalidMaxSz++) {
      buffer = kaBuffer.data();
      sz = 0;
      result = serializer.serialize(&buffer, &sz, invalidMaxSz, SerializeIF::Endianness::NETWORK);
      REQUIRE(result != returnvalue::OK);
    }
  }

  SECTION("Deserialize") {
    KeepAlivePduCreator serializer(pduConf, progress);
    result = serializer.serialize(&buffer, &sz, kaBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);

    // Set another file size
    progress.setFileSize(200, false);
    KeepAlivePduReader reader(kaBuffer.data(), kaBuffer.size(), progress);
    result = reader.parseData();
    REQUIRE(result == returnvalue::OK);
    auto& progRef = reader.getProgress();
    // Should have been overwritten
    REQUIRE(progRef.getSize() == 0x50);
    sz = reader.getWholePduSize();

    // invalid max size
    for (size_t invalidMaxSz = 0; invalidMaxSz < sz; invalidMaxSz++) {
      ReturnValue_t setResult = reader.setReadOnlyData(kaBuffer.data(), invalidMaxSz);
      if (setResult == returnvalue::OK) {
        result = reader.parseData();
        REQUIRE(result != returnvalue::OK);
      }
    }
  }
}
