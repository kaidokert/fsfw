#include <fsfw/cfdp/tlv/EntityIdTlv.h>
#include <fsfw/cfdp/tlv/FaultHandlerOverrideTlv.h>
#include <fsfw/cfdp/tlv/FilestoreRequestTlv.h>
#include <fsfw/cfdp/tlv/FilestoreResponseTlv.h>
#include <fsfw/cfdp/tlv/FlowLabelTlv.h>
#include <fsfw/cfdp/tlv/MessageToUserTlv.h>

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <string>

#include "fsfw/cfdp/pdu/PduConfig.h"
#include "fsfw/cfdp/tlv/Lv.h"
#include "fsfw/cfdp/tlv/Tlv.h"
#include "fsfw/globalfunctions/arrayprinter.h"

TEST_CASE("CFDP Other TLVs", "[cfdp][tlv]") {
  using namespace cfdp;
  ReturnValue_t result;
  std::array<uint8_t, 255> rawBuf{};
  uint8_t* serPtr = rawBuf.data();
  const uint8_t* deserPtr = rawBuf.data();
  size_t deserSize = 0;
  cfdp::EntityId sourceId = EntityId(cfdp::WidthInBytes::TWO_BYTES, 0x0ff0);

  SECTION("Filestore Response TLV") {
    std::string name = "hello.txt";
    cfdp::StringLv firstName(name);
    std::string name2 = "hello2.txt";
    cfdp::StringLv secondName(name2);
    std::string msg = "12345";
    cfdp::Lv fsMsg(reinterpret_cast<const uint8_t*>(msg.data()), msg.size());
    FilestoreResponseTlv response(cfdp::FilestoreActionCode::APPEND_FILE, cfdp::FSR_SUCCESS,
                                  firstName, &fsMsg);
    response.setSecondFileName(&secondName);
    REQUIRE(response.getLengthField() == 10 + 11 + 6 + 1);
    REQUIRE(response.getSerializedSize() == response.getLengthField() + 2);

    cfdp::Tlv rawResponse;
    std::array<uint8_t, 128> serBuf = {};
    result = response.convertToTlv(rawResponse, serBuf.data(), serBuf.size(),
                                   SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(rawResponse.getType() == cfdp::TlvType::FILESTORE_RESPONSE);
    cfdp::StringLv emptyMsg;
    cfdp::StringLv emptySecondName;
    FilestoreResponseTlv emptyTlv(firstName, &emptyMsg);
    emptyTlv.setSecondFileName(&emptySecondName);
    result = emptyTlv.deSerialize(rawResponse, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(emptyTlv.getActionCode() == cfdp::FilestoreActionCode::APPEND_FILE);
    REQUIRE(emptyTlv.getStatusCode() == cfdp::FSR_SUCCESS);
    size_t firstNameLen = 0;
    const char* firstNamePtr =
        reinterpret_cast<const char*>(emptyTlv.getFirstFileName().getValue(&firstNameLen));
    auto helloString = std::string(firstNamePtr, firstNameLen);
    REQUIRE(helloString == "hello.txt");
  }

  SECTION("Filestore Request TLV") {
    std::string name = "hello.txt";
    cfdp::StringLv firstName(name);
    std::string name2 = "hello2.txt";
    cfdp::StringLv secondName(name2);
    FilestoreRequestTlv request(cfdp::FilestoreActionCode::APPEND_FILE, firstName);

    // second name not set yet
    REQUIRE(request.getLengthField() == 10 + 1);
    REQUIRE(request.getSerializedSize() == request.getLengthField() + 2);

    std::array<uint8_t, 128> serBuf = {};
    uint8_t* ptr = serBuf.data();
    size_t sz = 0;
    result = request.serialize(&ptr, &sz, serBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == cfdp::FILESTORE_REQUIRES_SECOND_FILE);

    ptr = serBuf.data();
    sz = 0;
    request.setSecondFileName(&secondName);
    size_t expectedSz = request.getLengthField();
    REQUIRE(expectedSz == 10 + 11 + 1);
    REQUIRE(request.getSerializedSize() == expectedSz + 2);
    result = request.serialize(&ptr, &sz, serBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(sz == expectedSz + 2);

    FilestoreRequestTlv emptyRequest(firstName);
    emptyRequest.setSecondFileName(&secondName);
    const uint8_t* constptr = serBuf.data();
    result = emptyRequest.deSerialize(&constptr, &sz, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);

    cfdp::Tlv rawRequest;
    ptr = serBuf.data();
    sz = 0;
    result = request.convertToTlv(rawRequest, serBuf.data(), serBuf.size(),
                                  SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(rawRequest.getType() == cfdp::TlvType::FILESTORE_REQUEST);

    emptyRequest.setActionCode(cfdp::FilestoreActionCode::DELETE_FILE);
    result = emptyRequest.deSerialize(rawRequest, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(emptyRequest.getType() == cfdp::TlvType::FILESTORE_REQUEST);
    REQUIRE(emptyRequest.getActionCode() == cfdp::FilestoreActionCode::APPEND_FILE);
  }

  SECTION("Other") {
    MessageToUserTlv emptyTlv;
    uint8_t flowLabel = 1;
    FlowLabelTlv flowLabelTlv(&flowLabel, 1);

    FaultHandlerOverrideTlv faultOverrideTlv(cfdp::ConditionCode::FILESTORE_REJECTION,
                                             cfdp::FaultHandlerCode::NOTICE_OF_CANCELLATION);
    size_t sz = 0;
    result =
        faultOverrideTlv.serialize(&serPtr, &sz, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(faultOverrideTlv.getSerializedSize() == 3);
    REQUIRE(sz == 3);
    REQUIRE(result == returnvalue::OK);

    FaultHandlerOverrideTlv emptyOverrideTlv;
    result = emptyOverrideTlv.deSerialize(&deserPtr, &sz, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);

    EntityId entId(cfdp::WidthInBytes::TWO_BYTES, 0x42);
    EntityId emptyId;
    EntityIdTlv idTlv(emptyId);
    serPtr = rawBuf.data();
    result = idTlv.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    cfdp::Tlv rawTlv(cfdp::TlvType::ENTITY_ID, rawBuf.data() + 2, 2);
    REQUIRE(result == returnvalue::OK);
    deserPtr = rawBuf.data();
    result = idTlv.deSerialize(rawTlv, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
  }
}
