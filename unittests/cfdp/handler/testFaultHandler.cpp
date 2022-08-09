#include <catch2/catch_test_macros.hpp>

#include "mocks/CfdpFaultHandlerMock.h"

TEST_CASE("CFDP Fault Handler", "[cfdp]") {
  using namespace cfdp;
  auto fhMock = CfdpFaultHandlerMock();
  cfdp::FaultHandlerCodes fhCode;
  SECTION("State") {
    // Verify initial condition
    CHECK(fhMock.getFaultHandler(ConditionCode::UNSUPPORTED_CHECKSUM_TYPE, fhCode));
    CHECK(fhCode == FaultHandlerCodes::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::POSITIVE_ACK_LIMIT_REACHED, fhCode));
    CHECK(fhCode == FaultHandlerCodes::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::KEEP_ALIVE_LIMIT_REACHED, fhCode));
    CHECK(fhCode == FaultHandlerCodes::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::INVALID_TRANSMISSION_MODE, fhCode));
    CHECK(fhCode == FaultHandlerCodes::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::FILESTORE_REJECTION, fhCode));
    CHECK(fhCode == FaultHandlerCodes::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::FILE_CHECKSUM_FAILURE, fhCode));
    CHECK(fhCode == FaultHandlerCodes::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::FILE_SIZE_ERROR, fhCode));
    CHECK(fhCode == FaultHandlerCodes::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::NAK_LIMIT_REACHED, fhCode));
    CHECK(fhCode == FaultHandlerCodes::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::INACTIVITY_DETECTED, fhCode));
    CHECK(fhCode == FaultHandlerCodes::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::CHECK_LIMIT_REACHED, fhCode));
    CHECK(fhCode == FaultHandlerCodes::IGNORE_ERROR);
  }

  SECTION("Call Handler") {
    fhMock.reportFault(ConditionCode::CHECK_LIMIT_REACHED);
  }
}