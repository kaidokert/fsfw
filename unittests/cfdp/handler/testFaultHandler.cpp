#include <catch2/catch_test_macros.hpp>

#include "mocks/cfdp/FaultHandlerMock.h"

TEST_CASE("CFDP Fault Handler", "[cfdp]") {
  using namespace cfdp;
  auto fhMock = FaultHandlerMock();
  cfdp::FaultHandlerCode fhCode;
  cfdp::TransactionId id;

  SECTION("State") {
    // Verify initial condition
    CHECK(fhMock.getFaultHandler(ConditionCode::UNSUPPORTED_CHECKSUM_TYPE, fhCode));
    CHECK(fhCode == FaultHandlerCode::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::POSITIVE_ACK_LIMIT_REACHED, fhCode));
    CHECK(fhCode == FaultHandlerCode::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::KEEP_ALIVE_LIMIT_REACHED, fhCode));
    CHECK(fhCode == FaultHandlerCode::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::INVALID_TRANSMISSION_MODE, fhCode));
    CHECK(fhCode == FaultHandlerCode::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::FILESTORE_REJECTION, fhCode));
    CHECK(fhCode == FaultHandlerCode::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::FILE_CHECKSUM_FAILURE, fhCode));
    CHECK(fhCode == FaultHandlerCode::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::FILE_SIZE_ERROR, fhCode));
    CHECK(fhCode == FaultHandlerCode::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::NAK_LIMIT_REACHED, fhCode));
    CHECK(fhCode == FaultHandlerCode::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::INACTIVITY_DETECTED, fhCode));
    CHECK(fhCode == FaultHandlerCode::IGNORE_ERROR);
    CHECK(fhMock.getFaultHandler(ConditionCode::CHECK_LIMIT_REACHED, fhCode));
    CHECK(fhCode == FaultHandlerCode::IGNORE_ERROR);
  }

  SECTION("Call Handler, Ignore Fault") {
    auto& info = fhMock.getFhInfo(FaultHandlerCode::IGNORE_ERROR);
    CHECK(fhMock.reportFault(id, ConditionCode::CHECK_LIMIT_REACHED));
    CHECK(info.callCount == 1);
    CHECK(info.condCodes.back() == ConditionCode::CHECK_LIMIT_REACHED);
    fhMock.reportFault(id, ConditionCode::FILE_CHECKSUM_FAILURE);
    CHECK(info.callCount == 2);
    CHECK(info.condCodes.back() == ConditionCode::FILE_CHECKSUM_FAILURE);
  }

  SECTION("Invalid Reported Code") { CHECK(not fhMock.reportFault(id, ConditionCode::NO_ERROR)); }

  SECTION("Invalid FH code") {
    CHECK(not fhMock.setFaultHandler(ConditionCode::KEEP_ALIVE_LIMIT_REACHED,
                                     FaultHandlerCode::RESERVED));
    CHECK(fhMock.getFaultHandler(ConditionCode::KEEP_ALIVE_LIMIT_REACHED, fhCode));
    CHECK(fhCode == FaultHandlerCode::IGNORE_ERROR);
    CHECK(not fhMock.setFaultHandler(ConditionCode::NO_ERROR, FaultHandlerCode::IGNORE_ERROR));
    CHECK(not fhMock.getFaultHandler(ConditionCode::NO_ERROR, fhCode));
  }

  SECTION("Set Other Fault Handler") {
    CHECK(fhMock.setFaultHandler(ConditionCode::FILE_CHECKSUM_FAILURE,
                                 FaultHandlerCode::NOTICE_OF_CANCELLATION));
    CHECK(fhMock.setFaultHandler(ConditionCode::INACTIVITY_DETECTED,
                                 FaultHandlerCode::ABANDON_TRANSACTION));
    CHECK(fhMock.setFaultHandler(ConditionCode::KEEP_ALIVE_LIMIT_REACHED,
                                 FaultHandlerCode::NOTICE_OF_SUSPENSION));
    auto& ignoreInfo = fhMock.getFhInfo(FaultHandlerCode::IGNORE_ERROR);
    auto& cancellationInfo = fhMock.getFhInfo(FaultHandlerCode::NOTICE_OF_CANCELLATION);
    auto& suspensionInfo = fhMock.getFhInfo(FaultHandlerCode::NOTICE_OF_SUSPENSION);
    auto& abandonInfo = fhMock.getFhInfo(FaultHandlerCode::ABANDON_TRANSACTION);

    CHECK(fhMock.reportFault(id, ConditionCode::FILE_CHECKSUM_FAILURE));
    CHECK(cancellationInfo.callCount == 1);
    CHECK(cancellationInfo.condCodes.back() == ConditionCode::FILE_CHECKSUM_FAILURE);
    CHECK(ignoreInfo.callCount == 0);
    CHECK(suspensionInfo.callCount == 0);
    CHECK(abandonInfo.callCount == 0);

    CHECK(fhMock.reportFault(id, ConditionCode::INACTIVITY_DETECTED));
    CHECK(cancellationInfo.callCount == 1);
    CHECK(ignoreInfo.callCount == 0);
    CHECK(suspensionInfo.callCount == 0);
    CHECK(abandonInfo.callCount == 1);
    CHECK(abandonInfo.condCodes.back() == ConditionCode::INACTIVITY_DETECTED);

    CHECK(fhMock.reportFault(id, ConditionCode::KEEP_ALIVE_LIMIT_REACHED));
    CHECK(cancellationInfo.callCount == 1);
    CHECK(ignoreInfo.callCount == 0);
    CHECK(suspensionInfo.callCount == 1);
    CHECK(suspensionInfo.condCodes.back() == ConditionCode::KEEP_ALIVE_LIMIT_REACHED);
    CHECK(abandonInfo.callCount == 1);
  }
}