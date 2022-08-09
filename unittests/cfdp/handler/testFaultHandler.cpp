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

  SECTION("Call Handler, Ignore Fault") {
    auto& info = fhMock.getFhInfo(FaultHandlerCodes::IGNORE_ERROR);
    CHECK(fhMock.reportFault(ConditionCode::CHECK_LIMIT_REACHED));
    CHECK(info.callCount == 1);
    CHECK(info.condCodes.back() == ConditionCode::CHECK_LIMIT_REACHED);
    fhMock.reportFault(ConditionCode::FILE_CHECKSUM_FAILURE);
    CHECK(info.callCount == 2);
    CHECK(info.condCodes.back() == ConditionCode::FILE_CHECKSUM_FAILURE);
  }

  SECTION("Invalid Reported Code") { CHECK(not fhMock.reportFault(ConditionCode::NO_ERROR)); }

  SECTION("Invalid FH code"){
      CHECK(not fhMock.setFaultHandler(ConditionCode::KEEP_ALIVE_LIMIT_REACHED, FaultHandlerCodes::RESERVED));
      CHECK(fhMock.getFaultHandler(ConditionCode::KEEP_ALIVE_LIMIT_REACHED, fhCode));
      CHECK(fhCode == FaultHandlerCodes::IGNORE_ERROR);
      CHECK(not fhMock.setFaultHandler(ConditionCode::NO_ERROR, FaultHandlerCodes::IGNORE_ERROR));
      CHECK(not fhMock.getFaultHandler(ConditionCode::NO_ERROR, fhCode));
  }

  SECTION("Set Other Fault Handler") {
    CHECK(fhMock.setFaultHandler(ConditionCode::FILE_CHECKSUM_FAILURE,
                                 FaultHandlerCodes::NOTICE_OF_CANCELLATION));
    CHECK(fhMock.setFaultHandler(ConditionCode::INACTIVITY_DETECTED,
                                 FaultHandlerCodes::ABANDON_TRANSACTION));
    CHECK(fhMock.setFaultHandler(ConditionCode::KEEP_ALIVE_LIMIT_REACHED,
                                 FaultHandlerCodes::NOTICE_OF_SUSPENSION));
    auto& ignoreInfo = fhMock.getFhInfo(FaultHandlerCodes::IGNORE_ERROR);
    auto& cancellationInfo = fhMock.getFhInfo(FaultHandlerCodes::NOTICE_OF_CANCELLATION);
    auto& suspensionInfo = fhMock.getFhInfo(FaultHandlerCodes::NOTICE_OF_SUSPENSION);
    auto& abandonInfo = fhMock.getFhInfo(FaultHandlerCodes::ABANDON_TRANSACTION);

    CHECK(fhMock.reportFault(ConditionCode::FILE_CHECKSUM_FAILURE));
    CHECK(cancellationInfo.callCount == 1);
    CHECK(cancellationInfo.condCodes.back() == ConditionCode::FILE_CHECKSUM_FAILURE);
    CHECK(ignoreInfo.callCount == 0);
    CHECK(suspensionInfo.callCount == 0);
    CHECK(abandonInfo.callCount == 0);

    CHECK(fhMock.reportFault(ConditionCode::INACTIVITY_DETECTED));
    CHECK(cancellationInfo.callCount == 1);
    CHECK(ignoreInfo.callCount == 0);
    CHECK(suspensionInfo.callCount == 0);
    CHECK(abandonInfo.callCount == 1);
    CHECK(abandonInfo.condCodes.back() == ConditionCode::INACTIVITY_DETECTED);

    CHECK(fhMock.reportFault(ConditionCode::KEEP_ALIVE_LIMIT_REACHED));
    CHECK(cancellationInfo.callCount == 1);
    CHECK(ignoreInfo.callCount == 0);
    CHECK(suspensionInfo.callCount == 1);
    CHECK(suspensionInfo.condCodes.back() == ConditionCode::KEEP_ALIVE_LIMIT_REACHED);
    CHECK(abandonInfo.callCount == 1);
  }
}