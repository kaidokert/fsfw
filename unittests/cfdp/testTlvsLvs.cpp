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

TEST_CASE("CFDP TLV LV", "[CfdpTlvLv]") {
  using namespace cfdp;
  int result = HasReturnvaluesIF::RETURN_OK;
  std::array<uint8_t, 255> rawBuf{};
  uint8_t* serPtr = rawBuf.data();
  const uint8_t* deserPtr = rawBuf.data();
  size_t deserSize = 0;
  cfdp::EntityId sourceId = EntityId(cfdp::WidthInBytes::TWO_BYTES, 0x0ff0);

  SECTION("TLV Serialization") {
    std::array<uint8_t, 8> tlvRawBuf{};
    serPtr = tlvRawBuf.data();
    result =
        sourceId.serialize(&serPtr, &deserSize, tlvRawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(deserSize == 2);
    auto tlv = Tlv(TlvTypes::ENTITY_ID, tlvRawBuf.data(), deserSize);
    REQUIRE(tlv.getSerializedSize() == 4);
    REQUIRE(tlv.getLengthField() == 2);
    serPtr = rawBuf.data();
    deserSize = 0;
    result = tlv.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(deserSize == 4);
    REQUIRE(rawBuf[0] == TlvTypes::ENTITY_ID);
    REQUIRE(rawBuf[1] == 2);
    uint16_t entityId = 0;
    SerializeAdapter::deSerialize(&entityId, rawBuf.data() + 2, &deserSize,
                                  SerializeIF::Endianness::NETWORK);
    REQUIRE(entityId == 0x0ff0);

    // Set new value
    sourceId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 12);
    serPtr = tlvRawBuf.data();
    deserSize = 0;
    result =
        sourceId.serialize(&serPtr, &deserSize, tlvRawBuf.size(), SerializeIF::Endianness::NETWORK);
    tlv.setValue(tlvRawBuf.data(), cfdp::WidthInBytes::FOUR_BYTES);
    serPtr = rawBuf.data();
    deserSize = 0;
    result = tlv.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(rawBuf[0] == TlvTypes::ENTITY_ID);
    REQUIRE(rawBuf[1] == 4);

    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

    serPtr = rawBuf.data();
    deserSize = 0;
    auto tlvInvalid = Tlv(cfdp::TlvTypes::INVALID_TLV, tlvRawBuf.data(), 0);
    REQUIRE(tlvInvalid.serialize(&serPtr, &deserSize, rawBuf.size(),
                                 SerializeIF::Endianness::NETWORK) != HasReturnvaluesIF::RETURN_OK);
    tlvInvalid = Tlv(cfdp::TlvTypes::ENTITY_ID, nullptr, 3);
    REQUIRE(tlvInvalid.serialize(&serPtr, &deserSize, rawBuf.size(),
                                 SerializeIF::Endianness::NETWORK) != HasReturnvaluesIF::RETURN_OK);
    REQUIRE(tlvInvalid.serialize(&serPtr, &deserSize, 0, SerializeIF::Endianness::NETWORK) !=
            HasReturnvaluesIF::RETURN_OK);
    REQUIRE(tlvInvalid.getSerializedSize() == 0);
    REQUIRE(tlvInvalid.serialize(nullptr, nullptr, 0, SerializeIF::Endianness::NETWORK) !=
            HasReturnvaluesIF::RETURN_OK);

    Tlv zeroLenField(TlvTypes::FAULT_HANDLER, nullptr, 0);
    REQUIRE(zeroLenField.getSerializedSize() == 2);
    serPtr = rawBuf.data();
    deserSize = 0;
    REQUIRE(zeroLenField.serialize(&serPtr, &deserSize, rawBuf.size(),
                                   SerializeIF::Endianness::NETWORK) ==
            HasReturnvaluesIF::RETURN_OK);
    REQUIRE(rawBuf[0] == TlvTypes::FAULT_HANDLER);
    REQUIRE(rawBuf[1] == 0);
  }

  SECTION("TLV Deserialization") {
    // Serialization was tested before, generate raw data now
    std::array<uint8_t, 8> tlvRawBuf;
    serPtr = tlvRawBuf.data();
    result =
        sourceId.serialize(&serPtr, &deserSize, tlvRawBuf.size(), SerializeIF::Endianness::NETWORK);
    auto tlvSerialization = Tlv(TlvTypes::ENTITY_ID, tlvRawBuf.data(), deserSize);
    serPtr = rawBuf.data();
    deserSize = 0;
    result = tlvSerialization.serialize(&serPtr, &deserSize, rawBuf.size(),
                                        SerializeIF::Endianness::NETWORK);
    Tlv tlv;
    deserPtr = rawBuf.data();
    result = tlv.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(tlv.getSerializedSize() == 4);
    REQUIRE(tlv.getType() == TlvTypes::ENTITY_ID);
    deserPtr = tlv.getValue();
    uint16_t entityId = 0;
    deserSize = 0;
    SerializeAdapter::deSerialize(&entityId, deserPtr, &deserSize,
                                  SerializeIF::Endianness::NETWORK);
    REQUIRE(entityId == 0x0ff0);

    REQUIRE(tlv.deSerialize(nullptr, nullptr, SerializeIF::Endianness::NETWORK) !=
            HasReturnvaluesIF::RETURN_OK);
    deserPtr = rawBuf.data();
    deserSize = 0;
    REQUIRE(tlv.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::NETWORK) ==
            SerializeIF::STREAM_TOO_SHORT);
    // Set invalid TLV
    rawBuf[0] = TlvTypes::INVALID_TLV;
    deserSize = 4;
    REQUIRE(tlv.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::NETWORK) !=
            HasReturnvaluesIF::RETURN_OK);

    Tlv zeroLenField(TlvTypes::FAULT_HANDLER, nullptr, 0);
    serPtr = rawBuf.data();
    deserSize = 0;
    REQUIRE(zeroLenField.serialize(&serPtr, &deserSize, rawBuf.size(),
                                   SerializeIF::Endianness::NETWORK) ==
            HasReturnvaluesIF::RETURN_OK);
    deserPtr = rawBuf.data();
    result = zeroLenField.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(zeroLenField.getSerializedSize() == 2);
    REQUIRE(deserSize == 0);
  }

  SECTION("LV Serialization") {
    std::array<uint8_t, 8> lvRawBuf;
    serPtr = lvRawBuf.data();
    result =
        sourceId.serialize(&serPtr, &deserSize, lvRawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(deserSize == 2);
    auto lv = cfdp::Lv(lvRawBuf.data(), 2);
    auto lvCopy = cfdp::Lv(lv);
    REQUIRE(lv.getSerializedSize() == 3);
    REQUIRE(lvCopy.getSerializedSize() == 3);
    REQUIRE(lv.getValue(nullptr) == lvCopy.getValue(nullptr));
    serPtr = rawBuf.data();
    deserSize = 0;
    result = lv.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(deserSize == 3);
    REQUIRE(rawBuf[0] == 2);
    uint16_t sourceId = 0;
    result = SerializeAdapter::deSerialize(&sourceId, rawBuf.data() + 1, &deserSize,
                                           SerializeIF::Endianness::BIG);
    REQUIRE(sourceId == 0x0ff0);

    auto lvEmpty = Lv(nullptr, 0);
    REQUIRE(lvEmpty.getSerializedSize() == 1);
    serPtr = rawBuf.data();
    deserSize = 0;
    result =
        lvEmpty.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(deserSize == 1);
  }

  SECTION("LV Deserialization") {
    std::array<uint8_t, 8> lvRawBuf;
    serPtr = lvRawBuf.data();
    result =
        sourceId.serialize(&serPtr, &deserSize, lvRawBuf.size(), SerializeIF::Endianness::NETWORK);
    auto lv = cfdp::Lv(lvRawBuf.data(), 2);
    serPtr = rawBuf.data();
    deserSize = 0;
    result = lv.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

    Lv uninitLv;
    deserPtr = rawBuf.data();
    deserSize = 3;
    result = uninitLv.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::BIG);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(uninitLv.getSerializedSize() == 3);
    const uint8_t* storedValue = uninitLv.getValue(nullptr);
    uint16_t sourceId = 0;
    result = SerializeAdapter::deSerialize(&sourceId, storedValue, &deserSize,
                                           SerializeIF::Endianness::BIG);
    REQUIRE(sourceId == 0x0ff0);

    auto lvEmpty = Lv(nullptr, 0);
    REQUIRE(lvEmpty.getSerializedSize() == 1);
    serPtr = rawBuf.data();
    deserSize = 0;
    result =
        lvEmpty.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(deserSize == 1);
    deserPtr = rawBuf.data();
    result = uninitLv.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::BIG);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(uninitLv.getSerializedSize() == 1);

    REQUIRE(uninitLv.deSerialize(nullptr, nullptr, SerializeIF::Endianness::BIG) ==
            HasReturnvaluesIF::RETURN_FAILED);
    serPtr = rawBuf.data();
    deserSize = 0;
    REQUIRE(uninitLv.serialize(&serPtr, &deserSize, 0, SerializeIF::Endianness::BIG) ==
            SerializeIF::BUFFER_TOO_SHORT);
    REQUIRE(uninitLv.serialize(nullptr, nullptr, 12, SerializeIF::Endianness::BIG));
    deserSize = 0;
    REQUIRE(uninitLv.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::BIG) ==
            SerializeIF::STREAM_TOO_SHORT);
  }

  SECTION("Filestore Response TLV") {
    std::string name = "hello.txt";
    cfdp::Lv firstName(reinterpret_cast<const uint8_t*>(name.data()), name.size());
    std::string name2 = "hello2.txt";
    cfdp::Lv secondName(reinterpret_cast<const uint8_t*>(name2.data()), name2.size());
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
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(rawResponse.getType() == cfdp::TlvTypes::FILESTORE_RESPONSE);
    cfdp::Lv emptyMsg;
    cfdp::Lv emptySecondName;
    FilestoreResponseTlv emptyTlv(firstName, &emptyMsg);
    emptyTlv.setSecondFileName(&emptySecondName);
    result = emptyTlv.deSerialize(rawResponse, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
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
    cfdp::Lv firstName(reinterpret_cast<const uint8_t*>(name.data()), name.size());
    std::string name2 = "hello2.txt";
    cfdp::Lv secondName(reinterpret_cast<const uint8_t*>(name2.data()), name2.size());
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
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(sz == expectedSz + 2);

    FilestoreRequestTlv emptyRequest(firstName);
    emptyRequest.setSecondFileName(&secondName);
    const uint8_t* constptr = serBuf.data();
    result = emptyRequest.deSerialize(&constptr, &sz, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

    cfdp::Tlv rawRequest;
    ptr = serBuf.data();
    sz = 0;
    result = request.convertToTlv(rawRequest, serBuf.data(), serBuf.size(),
                                  SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(rawRequest.getType() == cfdp::TlvTypes::FILESTORE_REQUEST);

    emptyRequest.setActionCode(cfdp::FilestoreActionCode::DELETE_FILE);
    result = emptyRequest.deSerialize(rawRequest, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(emptyRequest.getType() == cfdp::TlvTypes::FILESTORE_REQUEST);
    REQUIRE(emptyRequest.getActionCode() == cfdp::FilestoreActionCode::APPEND_FILE);
  }

  SECTION("Other") {
    MessageToUserTlv emptyTlv;
    uint8_t flowLabel = 1;
    FlowLabelTlv flowLabelTlv(&flowLabel, 1);

    FaultHandlerOverrideTlv faultOverrideTlv(cfdp::ConditionCode::FILESTORE_REJECTION,
                                             cfdp::FaultHandlerCode::NOTICE_OF_CANCELLATION);
    size_t sz = 0;
    ReturnValue_t result =
        faultOverrideTlv.serialize(&serPtr, &sz, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(faultOverrideTlv.getSerializedSize() == 3);
    REQUIRE(sz == 3);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

    FaultHandlerOverrideTlv emptyOverrideTlv;
    result = emptyOverrideTlv.deSerialize(&deserPtr, &sz, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

    EntityId entId(cfdp::WidthInBytes::TWO_BYTES, 0x42);
    EntityId emptyId;
    EntityIdTlv idTlv(emptyId);
    serPtr = rawBuf.data();
    result = idTlv.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    cfdp::Tlv rawTlv(cfdp::TlvTypes::ENTITY_ID, rawBuf.data() + 2, 2);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    deserPtr = rawBuf.data();
    result = idTlv.deSerialize(rawTlv, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
  }
}
