#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/pdu/FinishedPduCreator.h"
#include "fsfw/cfdp/pdu/FinishedPduReader.h"
#include "fsfw/globalfunctions/arrayprinter.h"

TEST_CASE("Finished PDU", "[cfdp][pdu]") {
  using namespace cfdp;
  ReturnValue_t result = returnvalue::OK;
  std::array<uint8_t, 256> fnBuffer = {};
  uint8_t* buffer = fnBuffer.data();
  size_t sz = 0;
  EntityId destId(WidthInBytes::TWO_BYTES, 2);
  TransactionSeqNum seqNum(WidthInBytes::TWO_BYTES, 15);
  EntityId sourceId(WidthInBytes::TWO_BYTES, 1);
  PduConfig pduConf(sourceId, destId, TransmissionMode::ACKNOWLEDGED, seqNum);

  cfdp::Lv emptyFsMsg;
  FinishedInfo info(cfdp::ConditionCode::INACTIVITY_DETECTED,
                    cfdp::FileDeliveryCode::DATA_INCOMPLETE,
                    cfdp::FileDeliveryStatus::DISCARDED_DELIBERATELY);

  SECTION("Serialize") {
    FinishPduCreator serializer(pduConf, info);
    result = serializer.serialize(&buffer, &sz, fnBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(serializer.getSerializedSize() == 12);
    REQUIRE(((fnBuffer[1] << 8) | fnBuffer[2]) == 2);
    REQUIRE(fnBuffer[10] == cfdp::FileDirective::FINISH);
    REQUIRE(((fnBuffer[sz - 1] >> 4) & 0x0f) == cfdp::ConditionCode::INACTIVITY_DETECTED);
    REQUIRE(((fnBuffer[sz - 1] >> 2) & 0x01) == cfdp::FileDeliveryCode::DATA_INCOMPLETE);
    REQUIRE((fnBuffer[sz - 1] & 0b11) == cfdp::FileDeliveryStatus::DISCARDED_DELIBERATELY);
    REQUIRE(sz == 12);

    // Add a filestore response
    std::string firstName = "hello.txt";
    cfdp::StringLv firstNameLv(firstName);
    FilestoreResponseTlv response(cfdp::FilestoreActionCode::DELETE_FILE,
                                  cfdp::FSR_APPEND_FILE_1_NOT_EXISTS, firstNameLv, nullptr);
    FilestoreResponseTlv* responsePtr = &response;
    REQUIRE(response.getSerializedSize() == 14);
    size_t len = 1;
    info.setFilestoreResponsesArray(&responsePtr, &len, &len);
    serializer.updateDirectiveFieldLen();
    REQUIRE(serializer.getSerializedSize() == 12 + 14);
    sz = 0;
    buffer = fnBuffer.data();
    result = serializer.serialize(&buffer, &sz, fnBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(serializer.getSerializedSize() == 12 + 14);
    REQUIRE(serializer.getPduDataFieldLen() == 16);

    // Add two filestore responses and a fault location parameter
    std::string secondName = "hello2.txt";
    cfdp::StringLv secondNameLv(secondName);
    FilestoreResponseTlv response2(cfdp::FilestoreActionCode::DENY_FILE, cfdp::FSR_SUCCESS,
                                   secondNameLv, nullptr);
    REQUIRE(response2.getSerializedSize() == 15);
    len = 2;
    std::array<FilestoreResponseTlv*, 2> responses{&response, &response2};
    info.setFilestoreResponsesArray(responses.data(), &len, &len);
    serializer.updateDirectiveFieldLen();

    EntityIdTlv faultLoc(destId);
    REQUIRE(faultLoc.getSerializedSize() == 4);
    info.setFaultLocation(&faultLoc);
    serializer.updateDirectiveFieldLen();
    sz = 0;
    buffer = fnBuffer.data();
    result = serializer.serialize(&buffer, &sz, fnBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    info.setConditionCode(cfdp::ConditionCode::FILESTORE_REJECTION);
    REQUIRE(serializer.getSerializedSize() == 12 + 14 + 15 + 4);
    REQUIRE(sz == 12 + 14 + 15 + 4);
    info.setFileStatus(cfdp::FileDeliveryStatus::DISCARDED_FILESTORE_REJECTION);
    REQUIRE(info.getFileStatus() == cfdp::FileDeliveryStatus::DISCARDED_FILESTORE_REJECTION);
    info.setDeliveryCode(cfdp::FileDeliveryCode::DATA_INCOMPLETE);
    REQUIRE(info.getDeliveryCode() == cfdp::FileDeliveryCode::DATA_INCOMPLETE);
    for (size_t maxSz = 0; maxSz < 45; maxSz++) {
      sz = 0;
      buffer = fnBuffer.data();
      result = serializer.serialize(&buffer, &sz, maxSz, SerializeIF::Endianness::NETWORK);
      REQUIRE(result != returnvalue::OK);
    }
  }

  SECTION("Deserialize") {
    FinishedInfo emptyInfo;
    FinishPduCreator serializer(pduConf, info);
    result = serializer.serialize(&buffer, &sz, fnBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    FinishPduReader deserializer(fnBuffer.data(), fnBuffer.size(), emptyInfo);
    result = deserializer.parseData();
    REQUIRE(result == returnvalue::OK);
    REQUIRE(emptyInfo.getFileStatus() == cfdp::FileDeliveryStatus::DISCARDED_DELIBERATELY);
    REQUIRE(emptyInfo.getConditionCode() == cfdp::ConditionCode::INACTIVITY_DETECTED);
    REQUIRE(emptyInfo.getDeliveryCode() == cfdp::FileDeliveryCode::DATA_INCOMPLETE);

    // Add a filestore response
    sz = 0;
    buffer = fnBuffer.data();
    std::string firstName = "hello.txt";
    cfdp::StringLv firstNameLv(firstName);
    FilestoreResponseTlv response(cfdp::FilestoreActionCode::DELETE_FILE, cfdp::FSR_NOT_PERFORMED,
                                  firstNameLv, nullptr);
    FilestoreResponseTlv* responsePtr = &response;
    size_t len = 1;
    info.setFilestoreResponsesArray(&responsePtr, &len, &len);
    serializer.updateDirectiveFieldLen();
    REQUIRE(serializer.getPduDataFieldLen() == 16);
    result = serializer.serialize(&buffer, &sz, fnBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    FilestoreResponseTlv emptyResponse(firstNameLv, nullptr);
    responsePtr = &emptyResponse;
    emptyInfo.setFilestoreResponsesArray(&responsePtr, nullptr, &len);
    FinishPduReader deserializer2(fnBuffer.data(), fnBuffer.size(), emptyInfo);
    result = deserializer2.parseData();
    REQUIRE(result == returnvalue::OK);
    REQUIRE(emptyInfo.getFsResponsesLen() == 1);
    FilestoreResponseTlv** responseArray = nullptr;
    emptyInfo.getFilestoreResonses(&responseArray, nullptr, nullptr);
    REQUIRE(responseArray[0]->getActionCode() == cfdp::FilestoreActionCode::DELETE_FILE);
    REQUIRE(responseArray[0]->getStatusCode() == cfdp::FSR_NOT_PERFORMED);
    auto& fileNameRef = responseArray[0]->getFirstFileName();
    size_t stringSize = 0;
    const char* string = reinterpret_cast<const char*>(fileNameRef.getValue(&stringSize));
    std::string firstFileName(string, stringSize);
    REQUIRE(firstFileName == "hello.txt");

    // Add two filestore responses and a fault location parameter
    std::string secondName = "hello2.txt";
    cfdp::StringLv secondNameLv(secondName);
    FilestoreResponseTlv response2(cfdp::FilestoreActionCode::DENY_FILE, cfdp::FSR_SUCCESS,
                                   secondNameLv, nullptr);
    REQUIRE(response2.getSerializedSize() == 15);
    len = 2;
    std::array<FilestoreResponseTlv*, 2> responses{&response, &response2};
    info.setFilestoreResponsesArray(responses.data(), &len, &len);
    serializer.updateDirectiveFieldLen();

    EntityIdTlv faultLoc(destId);
    info.setFaultLocation(&faultLoc);
    serializer.updateDirectiveFieldLen();
    sz = 0;
    buffer = fnBuffer.data();
    result = serializer.serialize(&buffer, &sz, fnBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    EntityId emptyId;
    EntityIdTlv emptyFaultLoc(emptyId);
    emptyInfo.setFaultLocation(&emptyFaultLoc);
    response.setFilestoreMessage(&emptyFsMsg);
    emptyInfo.setFilestoreResponsesArray(responses.data(), &len, &len);
    response2.setFilestoreMessage(&emptyFsMsg);
    FinishPduReader deserializer3(fnBuffer.data(), fnBuffer.size(), emptyInfo);
    result = deserializer3.parseData();
    REQUIRE(result == returnvalue::OK);
    auto& infoRef = deserializer3.getInfo();
    REQUIRE(deserializer3.getWholePduSize() == 45);

    size_t invalidMaxLen = 1;
    emptyInfo.setFilestoreResponsesArray(responses.data(), &len, &invalidMaxLen);
    result = deserializer3.parseData();
    REQUIRE(result == cfdp::FINISHED_CANT_PARSE_FS_RESPONSES);
    emptyInfo.setFilestoreResponsesArray(nullptr, nullptr, nullptr);
    result = deserializer3.parseData();
    REQUIRE(result == cfdp::FINISHED_CANT_PARSE_FS_RESPONSES);

    // Clear condition code
    auto tmp = fnBuffer[11];
    fnBuffer[11] = fnBuffer[11] & ~0xf0;
    fnBuffer[11] = fnBuffer[11] | (cfdp::ConditionCode::NO_ERROR << 4);
    emptyInfo.setFilestoreResponsesArray(responses.data(), &len, &len);
    result = deserializer3.parseData();
    REQUIRE(result == cfdp::INVALID_TLV_TYPE);

    fnBuffer[11] = tmp;
    // Invalid TLV type, should be entity ID
    fnBuffer[sz - 4] = cfdp::TlvType::FILESTORE_REQUEST;
    result = deserializer3.parseData();
    REQUIRE(result == cfdp::INVALID_TLV_TYPE);

    for (size_t maxSz = 0; maxSz < 45; maxSz++) {
      FinishPduReader faultyDeser(fnBuffer.data(), maxSz, emptyInfo);
      result = faultyDeser.parseData();
      REQUIRE(result != returnvalue::OK);
    }
  }
}
