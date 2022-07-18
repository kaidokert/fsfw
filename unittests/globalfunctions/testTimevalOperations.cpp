#include <fsfw/globalfunctions/timevalOperations.h>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"

TEST_CASE("TimevalTest", "[timevalOperations]") {
  SECTION("Comparison") {
    timeval t1;
    t1.tv_sec = 1648227422;
    t1.tv_usec = 123456;
    timeval t2;
    t2.tv_sec = 1648227422;
    t2.tv_usec = 123456;
    REQUIRE(t1 == t2);
    REQUIRE(t2 == t1);
    REQUIRE_FALSE(t1 != t2);
    REQUIRE_FALSE(t2 != t1);
    REQUIRE(t1 <= t2);
    REQUIRE(t2 <= t1);
    REQUIRE(t1 >= t2);
    REQUIRE(t2 >= t1);
    REQUIRE_FALSE(t1 < t2);
    REQUIRE_FALSE(t2 < t1);
    REQUIRE_FALSE(t1 > t2);
    REQUIRE_FALSE(t2 > t1);

    timeval t3;
    t3.tv_sec = 1648227422;
    t3.tv_usec = 123457;
    REQUIRE_FALSE(t1 == t3);
    REQUIRE(t1 != t3);
    REQUIRE(t1 <= t3);
    REQUIRE_FALSE(t3 <= t1);
    REQUIRE_FALSE(t1 >= t3);
    REQUIRE(t3 >= t1);
    REQUIRE(t1 < t3);
    REQUIRE_FALSE(t3 < t1);
    REQUIRE_FALSE(t1 > t3);
    REQUIRE(t3 > t1);

    timeval t4;
    t4.tv_sec = 1648227423;
    t4.tv_usec = 123456;
    REQUIRE_FALSE(t1 == t4);
    REQUIRE(t1 != t4);
    REQUIRE(t1 <= t4);
    REQUIRE_FALSE(t4 <= t1);
    REQUIRE_FALSE(t1 >= t4);
    REQUIRE(t4 >= t1);
    REQUIRE(t1 < t4);
    REQUIRE_FALSE(t4 < t1);
    REQUIRE_FALSE(t1 > t4);
    REQUIRE(t4 > t1);
  }
  SECTION("Operators") {
    timeval t1;
    t1.tv_sec = 1648227422;
    t1.tv_usec = 123456;
    timeval t2;
    t2.tv_sec = 1648227422;
    t2.tv_usec = 123456;
    timeval t3 = t1 - t2;
    REQUIRE(t3.tv_sec == 0);
    REQUIRE(t3.tv_usec == 0);
    timeval t4 = t1 - t3;
    REQUIRE(t4.tv_sec == 1648227422);
    REQUIRE(t4.tv_usec == 123456);
    timeval t5 = t3 - t1;
    REQUIRE(t5.tv_sec == -1648227422);
    REQUIRE(t5.tv_usec == -123456);

    timeval t6;
    t6.tv_sec = 1648227400;
    t6.tv_usec = 999999;

    timeval t7 = t6 + t1;
    REQUIRE(t7.tv_sec == (1648227422ull + 1648227400ull + 1ull));
    REQUIRE(t7.tv_usec == 123455);

    timeval t8 = t1 - t6;
    REQUIRE(t8.tv_sec == 1648227422 - 1648227400 - 1);
    REQUIRE(t8.tv_usec == 123457);

    double scalar = 2;
    timeval t9 = t1 * scalar;
    REQUIRE(t9.tv_sec == 3296454844);
    REQUIRE(t9.tv_usec == 246912);
    timeval t10 = scalar * t1;
    REQUIRE(t10.tv_sec == 3296454844);
    REQUIRE(t10.tv_usec == 246912);
    timeval t11 = t6 * scalar;
    REQUIRE(t11.tv_sec == (3296454800 + 1));
    REQUIRE(t11.tv_usec == 999998);

    timeval t12 = t1 / scalar;
    REQUIRE(t12.tv_sec == 824113711);
    REQUIRE(t12.tv_usec == 61728);

    timeval t13 = t6 / scalar;
    REQUIRE(t13.tv_sec == 824113700);
    // Rounding issue
    REQUIRE(t13.tv_usec == 499999);

    double scalar2 = t9 / t1;
    REQUIRE(scalar2 == Catch::Approx(2.0));
    double scalar3 = t1 / t6;
    REQUIRE(scalar3 == Catch::Approx(1.000000013));
    double scalar4 = t3 / t1;
    REQUIRE(scalar4 == Catch::Approx(0));
    double scalar5 = t12 / t1;
    REQUIRE(scalar5 == Catch::Approx(0.5));
  }

  SECTION("timevalOperations::toTimeval") {
    double seconds = 1648227422.123456;
    timeval t1 = timevalOperations::toTimeval(seconds);
    REQUIRE(t1.tv_sec == 1648227422);
    // Allow 1 usec rounding tolerance
    REQUIRE(t1.tv_usec >= 123455);
    REQUIRE(t1.tv_usec <= 123457);
  }
}