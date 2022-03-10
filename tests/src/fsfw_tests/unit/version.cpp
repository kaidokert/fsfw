
#include "fsfw/version.h"

#include <catch2/catch_test_macros.hpp>

#include "fsfw/serviceinterface.h"
#include "fsfw_tests/unit/CatchDefinitions.h"

TEST_CASE("Version API Tests", "[TestVersionAPI]") {
  // Check that major version is non-zero
  REQUIRE(fsfw::FSFW_VERSION.major > 0);
  uint32_t fsfwMajor = fsfw::FSFW_VERSION.major;
  REQUIRE(fsfw::Version(255, 0, 0) > fsfw::FSFW_VERSION);
  REQUIRE(fsfw::Version(255, 0, 0) >= fsfw::FSFW_VERSION);
  REQUIRE(fsfw::Version(0, 0, 0) < fsfw::FSFW_VERSION);
  REQUIRE(fsfw::Version(0, 0, 0) <= fsfw::FSFW_VERSION);
  fsfw::Version v1 = fsfw::Version(1, 1, 1);
  fsfw::Version v2 = fsfw::Version(1, 1, 1);
  v1.revision -= 1;
  REQUIRE(v1 != v2);
  REQUIRE(v1 < v2);
  REQUIRE(v1 <= v2);
  v1.revision += 1;
  v1.minor -= 1;
  REQUIRE(v1 != v2);
  REQUIRE(v1 < v2);
  REQUIRE(v1 <= v2);
  v1.minor += 1;
  v1.major -= 1;
  REQUIRE(v1 != v2);
  REQUIRE(v1 < v2);
  REQUIRE(v1 <= v2);
  v1.major += 1;
  REQUIRE(v1 == v2);
  v1.major += 1;
  REQUIRE(v1 != v2);
  REQUIRE(v1 > v2);
  REQUIRE(v1 >= v2);
  v1.major -= 1;
  v1.minor += 1;
  REQUIRE(v1 != v2);
  REQUIRE(v1 > v2);
  REQUIRE(v1 >= v2);
  v1.minor -= 1;
  v1.revision += 1;
  REQUIRE(v1 != v2);
  REQUIRE(v1 > v2);
  REQUIRE(v1 >= v2);
  v1.revision -= 1;
  REQUIRE(v1 == v2);
  sif::info << "v" << fsfw::FSFW_VERSION << std::endl;
  char verString[10] = {};
  fsfw::FSFW_VERSION.getVersion(verString, sizeof(verString));
  sif::info << "v" << verString << std::endl;
}
