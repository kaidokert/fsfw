
#include "fsfw/version.h"

#include <catch2/catch_test_macros.hpp>

#include "fsfw/serviceinterface.h"
#include "fsfw_tests/unit/CatchDefinitions.h"

TEST_CASE("Version API Tests", "[TestVersionAPI]") {
  // Check that major version is non-zero
  REQUIRE(fsfw::FSFW_VERSION.major > 0);
  uint32_t fsfwMajor = fsfw::FSFW_VERSION.major;
  REQUIRE(Version(255, 0, 0) > fsfw::FSFW_VERSION);
  REQUIRE(Version(255, 0, 0) >= fsfw::FSFW_VERSION);
  REQUIRE(Version(0, 0, 0) < fsfw::FSFW_VERSION);
  REQUIRE(Version(0, 0, 0) <= fsfw::FSFW_VERSION);
  Version v1 = Version(1, 1, 1);
  Version v2 = Version(1, 1, 1);
  REQUIRE(v1 == v2);
  REQUIRE(not(v1 < v2));
  REQUIRE(not(v1 > v2));
  REQUIRE(v1 <= v2);
  REQUIRE(v1 >= v2);
  v1.revision -= 1;
  REQUIRE(v1 != v2);
  REQUIRE(not(v1 == v2));
  REQUIRE(not(v1 > v2));
  REQUIRE(not(v1 >= v2));
  REQUIRE(v1 < v2);
  REQUIRE(v1 <= v2);
  v1.revision += 1;
  v1.minor -= 1;
  REQUIRE(v1 != v2);
  REQUIRE(v1 < v2);
  REQUIRE(v1 <= v2);
  REQUIRE(not(v1 == v2));
  REQUIRE(not(v1 > v2));
  REQUIRE(not(v1 >= v2));
  v1.minor += 1;
  v1.major -= 1;
  REQUIRE(v1 != v2);
  REQUIRE(v1 < v2);
  REQUIRE(v1 <= v2);
  REQUIRE(not(v1 == v2));
  REQUIRE(not(v1 > v2));
  REQUIRE(not(v1 >= v2));
  v1.major += 1;
  REQUIRE(v1 == v2);
  REQUIRE(v1 <= v2);
  REQUIRE(v1 >= v2);
  REQUIRE(not(v1 != v2));
  REQUIRE(not(v1 > v2));
  REQUIRE(not(v1 < v2));
  v1.major += 1;
  v1.minor -= 1;
  REQUIRE(v1 != v2);
  REQUIRE(v1 > v2);
  REQUIRE(v1 >= v2);
  REQUIRE(not(v1 == v2));
  REQUIRE(not(v1 < v2));
  REQUIRE(not(v1 <= v2));
  v1.major -= 1;
  v1.minor += 2;
  v1.revision -= 1;
  REQUIRE(v1 != v2);
  REQUIRE(v1 > v2);
  REQUIRE(v1 >= v2);
  REQUIRE(not(v1 == v2));
  REQUIRE(not(v1 < v2));
  REQUIRE(not(v1 <= v2));
  v1.minor -= 1;
  v1.revision += 2;
  REQUIRE(v1 != v2);
  REQUIRE(v1 > v2);
  REQUIRE(v1 >= v2);
  REQUIRE(not(v1 == v2));
  REQUIRE(not(v1 < v2));
  REQUIRE(not(v1 <= v2));
  v1.revision -= 1;
  REQUIRE(v1 == v2);
  REQUIRE(v1 <= v2);
  REQUIRE(v1 >= v2);
  REQUIRE(not(v1 != v2));
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::info << "v" << fsfw::FSFW_VERSION << std::endl;
#endif
  char verString[10] = {};
  fsfw::FSFW_VERSION.getVersion(verString, sizeof(verString));
#if FSFW_DISABLE_PRINTOUT == 0
  printf("v%s\n", verString);
#endif
}
