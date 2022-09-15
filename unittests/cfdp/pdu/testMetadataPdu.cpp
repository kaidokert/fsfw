#include <array>
#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "fsfw/cfdp/pdu/MetadataPduCreator.h"
#include "fsfw/cfdp/pdu/MetadataPduReader.h"
#include "fsfw/cfdp/tlv/FilestoreResponseTlv.h"
#include "fsfw/cfdp/tlv/MessageToUserTlv.h"
#include "fsfw/globalfunctions/arrayprinter.h"

TEST_CASE("Metadata PDU", "[cfdp][pdu]") {
  using namespace cfdp;
  ReturnValue_t result = returnvalue::OK;
  std::array<uint8_t, 256> mdBuffer = {};
  uint8_t* buffer = mdBuffer.data();
  size_t sz = 0;
  EntityId destId(WidthInBytes::TWO_BYTES, 2);
  TransactionSeqNum seqNum(WidthInBytes::TWO_BYTES, 15);
  EntityId sourceId(WidthInBytes::TWO_BYTES, 1);
  PduConfig pduConf(sourceId, destId, TransmissionMode::ACKNOWLEDGED, seqNum);

  std::string firstFileName = "hello.txt";
  cfdp::StringLv sourceFileName(firstFileName);
  cfdp::StringLv destFileName;
  FileSize fileSize(35);
  MetadataInfo info(false, ChecksumType::MODULAR, fileSize, sourceFileName, destFileName);

  FilestoreResponseTlv response(FilestoreActionCode::CREATE_DIRECTORY, FSR_CREATE_NOT_ALLOWED,
                                sourceFileName, nullptr);
  std::array<uint8_t, 3> msg = {0x41, 0x42, 0x43};
  cfdp::Tlv responseTlv;
  std::array<uint8_t, 64> responseBuf = {};
  uint8_t* responseBufPtr = responseBuf.data();
  response.convertToTlv(responseTlv, buffer, responseBuf.size(), SerializeIF::Endianness::MACHINE);
  MessageToUserTlv msgToUser(msg.data(), msg.size());
  std::array<Tlv*, 2> options{&responseTlv, &msgToUser};
  REQUIRE(options[0]->getSerializedSize() == 2 + 1 + 10 + 1);
  REQUIRE(options[1]->getSerializedSize() == 5);

  SECTION("Serialize") {
    MetadataPduCreator serializer(pduConf, info);
    result = serializer.serialize(&buffer, &sz, mdBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(serializer.getWholePduSize() == 27);
    REQUIRE(info.getSourceFileName().getSerializedSize() == 10);
    REQUIRE(info.getDestFileName().getSerializedSize() == 1);
    REQUIRE(info.getSerializedSize() == 16);
    REQUIRE((mdBuffer[1] << 8 | mdBuffer[2]) == 17);
    REQUIRE(mdBuffer[10] == FileDirective::METADATA);
    // no closure requested and checksum type is modular => 0x00
    REQUIRE(mdBuffer[11] == 0x00);
    uint32_t fileSizeRaw = 0;
    result = SerializeAdapter::deSerialize(&fileSizeRaw, mdBuffer.data() + 12, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(fileSizeRaw == 35);
    REQUIRE(mdBuffer[16] == 9);
    REQUIRE(mdBuffer[17] == 'h');
    REQUIRE(mdBuffer[18] == 'e');
    REQUIRE(mdBuffer[19] == 'l');
    REQUIRE(mdBuffer[20] == 'l');
    REQUIRE(mdBuffer[21] == 'o');
    REQUIRE(mdBuffer[22] == '.');
    REQUIRE(mdBuffer[23] == 't');
    REQUIRE(mdBuffer[24] == 'x');
    REQUIRE(mdBuffer[25] == 't');
    REQUIRE(mdBuffer[26] == 0);

    std::string otherFileName = "hello2.txt";
    cfdp::StringLv otherFileNameLv(otherFileName.data(), otherFileName.size());
    info.setSourceFileName(otherFileNameLv);
    size_t sizeOfOptions = options.size();
    info.setOptionsArray(options.data(), sizeOfOptions, sizeOfOptions);
    REQUIRE(info.getMaxOptionsLen() == 2);
    info.setMaxOptionsLen(3);
    REQUIRE(info.getMaxOptionsLen() == 3);
    info.setChecksumType(cfdp::ChecksumType::CRC_32C);
    info.setClosureRequested(true);
    uint8_t* buffer = mdBuffer.data();
    size_t sz = 0;
    serializer.updateDirectiveFieldLen();

    result = serializer.serialize(&buffer, &sz, mdBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE((mdBuffer[1] << 8 | mdBuffer[2]) == 37);
    auto checksumType = static_cast<cfdp::ChecksumType>(mdBuffer[11] & 0x0f);
    REQUIRE(checksumType == cfdp::ChecksumType::CRC_32C);
    bool closureRequested = mdBuffer[11] >> 6 & 0x01;
    REQUIRE(closureRequested == true);
    // The size of the two options is 19. Summing up:
    //  - 11 bytes of source file name
    //  - 1 byte for dest file name
    //  - 4 for FSS
    //  - 1 leading byte.
    //  - 1 byte for PDU type
    // PDU header has 10 bytes.
    // I am not going to check the options raw content, those are part of the dedicated
    // TLV unittests
    REQUIRE(sz == 10 + 37);
    for (size_t maxSz = 0; maxSz < sz; maxSz++) {
      uint8_t* buffer = mdBuffer.data();
      size_t sz = 0;
      result = serializer.serialize(&buffer, &sz, maxSz, SerializeIF::Endianness::NETWORK);
      REQUIRE(result == SerializeIF::BUFFER_TOO_SHORT);
    }
    for (size_t initSz = 1; initSz < 47; initSz++) {
      uint8_t* buffer = mdBuffer.data();
      size_t sz = initSz;
      result = serializer.serialize(&buffer, &sz, 46, SerializeIF::Endianness::NETWORK);
      REQUIRE(result == SerializeIF::BUFFER_TOO_SHORT);
    }
    info.setDestFileName(destFileName);
  }

  SECTION("Deserialize") {
    MetadataPduCreator serializer(pduConf, info);
    result = serializer.serialize(&buffer, &sz, mdBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);

    MetadataPduReader deserializer(mdBuffer.data(), mdBuffer.size(), info);
    result = deserializer.parseData();
    REQUIRE(result == returnvalue::OK);
    size_t fullSize = deserializer.getWholePduSize();
    for (size_t maxSz = 0; maxSz < fullSize; maxSz++) {
      MetadataPduReader invalidSzDeser(mdBuffer.data(), maxSz, info);
      result = invalidSzDeser.parseData();
      REQUIRE(result != returnvalue::OK);
    }
    size_t sizeOfOptions = options.size();
    size_t maxSize = 4;
    info.setOptionsArray(options.data(), sizeOfOptions, maxSize);
    REQUIRE(info.getOptionsLen() == 2);
    info.setChecksumType(cfdp::ChecksumType::CRC_32C);
    info.setClosureRequested(true);
    uint8_t* buffer = mdBuffer.data();
    size_t sz = 0;
    serializer.updateDirectiveFieldLen();

    info.setSourceFileName(sourceFileName);
    result = serializer.serialize(&buffer, &sz, mdBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);

    MetadataPduReader deserializer2(mdBuffer.data(), mdBuffer.size(), info);
    result = deserializer2.parseData();
    REQUIRE(result == returnvalue::OK);
    REQUIRE(options[0]->getType() == cfdp::TlvType::FILESTORE_RESPONSE);
    REQUIRE(options[0]->getSerializedSize() == 14);
    REQUIRE(options[1]->getType() == cfdp::TlvType::MSG_TO_USER);
    REQUIRE(options[1]->getSerializedSize() == 5);

    for (size_t invalidFieldLen = 0; invalidFieldLen < 36; invalidFieldLen++) {
      mdBuffer[1] = (invalidFieldLen >> 8) & 0xff;
      mdBuffer[2] = invalidFieldLen & 0xff;
      result = deserializer2.parseData();
      if (invalidFieldLen == 17) {
        REQUIRE(info.getOptionsLen() == 0);
      }
      if (invalidFieldLen == 31) {
        REQUIRE(info.getOptionsLen() == 1);
      }
      // This is the precise length where there are no options or one option
      if (invalidFieldLen != 17 and invalidFieldLen != 31) {
        REQUIRE(result != returnvalue::OK);
      }
    }
    mdBuffer[1] = (36 >> 8) & 0xff;
    mdBuffer[2] = 36 & 0xff;
    info.setOptionsArray(nullptr, std::nullopt, std::nullopt);
    REQUIRE(deserializer2.parseData() == cfdp::METADATA_CANT_PARSE_OPTIONS);
    info.setOptionsArray(options.data(), sizeOfOptions, std::nullopt);
    for (size_t maxSz = 0; maxSz < 46; maxSz++) {
      MetadataPduReader invalidSzDeser(mdBuffer.data(), maxSz, info);
      if (not invalidSzDeser.isNull()) {
        result = invalidSzDeser.parseData();
        REQUIRE(result == SerializeIF::STREAM_TOO_SHORT);
      }
    }
  }
}
