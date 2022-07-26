#include <catch2/catch_test_macros.hpp>

#include "mocks/PusServiceBaseMock.h"
#include "mocks/PusVerificationReporterMock.h"

TEST_CASE("Pus Service Base", "[pus-service-base]") {
  auto verificationReporter = PusVerificationReporterMock();
  auto psb = PsbMock(17, 0x02, verificationReporter);
}