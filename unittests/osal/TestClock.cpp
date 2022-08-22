#include <fsfw/globalfunctions/timevalOperations.h>
#include <fsfw/timemanager/Clock.h>

#include <array>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"

TEST_CASE("OSAL::Clock Test", "[OSAL::Clock Test]") {
  SECTION("Test getClock") {
    timeval time;
    ReturnValue_t result = Clock::getClock_timeval(&time);
    REQUIRE(result == returnvalue::OK);
    Clock::TimeOfDay_t timeOfDay;
    result = Clock::getDateAndTime(&timeOfDay);
    REQUIRE(result == returnvalue::OK);
    timeval timeOfDayAsTimeval;
    result = Clock::convertTimeOfDayToTimeval(&timeOfDay, &timeOfDayAsTimeval);
    REQUIRE(result == returnvalue::OK);
    // We require timeOfDayAsTimeval to be larger than time as it
    // was request a few ns later
    double difference = timevalOperations::toDouble(timeOfDayAsTimeval - time);
    CHECK(difference >= 0.0);
    CHECK(difference <= 0.005);

    // Conversion in the other direction
    Clock::TimeOfDay_t timevalAsTimeOfDay;
    result = Clock::convertTimevalToTimeOfDay(&time, &timevalAsTimeOfDay);
    REQUIRE(result == returnvalue::OK);
    CHECK(timevalAsTimeOfDay.year <= timeOfDay.year);
    // TODO We should write TimeOfDay operators!
  }
  SECTION("Leap seconds") {
    uint16_t leapSeconds = 0;
    ReturnValue_t result = Clock::getLeapSeconds(&leapSeconds);
    REQUIRE(result == returnvalue::FAILED);
    REQUIRE(leapSeconds == 0);
    result = Clock::setLeapSeconds(18);
    REQUIRE(result == returnvalue::OK);
    result = Clock::getLeapSeconds(&leapSeconds);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(leapSeconds == 18);
  }
  SECTION("usec Test") {
    timeval timeAsTimeval;
    ReturnValue_t result = Clock::getClock_timeval(&timeAsTimeval);
    REQUIRE(result == returnvalue::OK);
    uint64_t timeAsUsec = 0;
    result = Clock::getClock_usecs(&timeAsUsec);
    REQUIRE(result == returnvalue::OK);
    double timeAsUsecDouble = static_cast<double>(timeAsUsec) / 1000000.0;
    timeval timeAsUsecTimeval = timevalOperations::toTimeval(timeAsUsecDouble);
    double difference = timevalOperations::toDouble(timeAsUsecTimeval - timeAsTimeval);
    // We accept 5 ms difference
    CHECK(difference >= 0.0);
    CHECK(difference <= 0.005);
    uint64_t timevalAsUint64 = static_cast<uint64_t>(timeAsTimeval.tv_sec) * 1000000ull +
                               static_cast<uint64_t>(timeAsTimeval.tv_usec);
    CHECK((timeAsUsec - timevalAsUint64) >= 0);
    CHECK((timeAsUsec - timevalAsUint64) <= (5 * 1000));
  }
  SECTION("Test j2000") {
    double j2000;
    timeval time;
    time.tv_sec = 1648208539;
    time.tv_usec = 0;
    ReturnValue_t result = Clock::convertTimevalToJD2000(time, &j2000);
    REQUIRE(result == returnvalue::OK);
    double correctJ2000 = 2459663.98772 - 2451545.0;
    CHECK(j2000 == Catch::Approx(correctJ2000).margin(1.2 * 1e-8));
  }
  SECTION("Convert to TT") {
    timeval utcTime;
    utcTime.tv_sec = 1648208539;
    utcTime.tv_usec = 999000;
    timeval tt;
    ReturnValue_t result = Clock::setLeapSeconds(27);
    REQUIRE(result == returnvalue::OK);
    result = Clock::convertUTCToTT(utcTime, &tt);
    REQUIRE(result == returnvalue::OK);
    CHECK(tt.tv_usec == 183000);
    // The plus 1 is a own forced overflow of usecs
    CHECK(tt.tv_sec == (1648208539 + 27 + 10 + 32 + 1));
  }
}