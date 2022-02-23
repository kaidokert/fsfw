#include <array>

#include <fsfw/timemanager/CCSDSTime.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "fsfw_tests/unit/CatchDefinitions.h"

TEST_CASE("CCSDSTime Tests", "[TestCCSDSTime]") {
  INFO("CCSDSTime Tests");
  CCSDSTime::Ccs_mseconds cssMilliSecconds;
  Clock::TimeOfDay_t time;
  time.year = 2020;
  time.month = 2;
  time.day = 29;
  time.hour = 13;
  time.minute = 24;
  time.second = 45;
  time.usecond = 123456;
  SECTION("Test CCS Time") {
    auto result = CCSDSTime::convertToCcsds(&cssMilliSecconds, &time);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(cssMilliSecconds.pField == 0x52);  // 0b01010010
    REQUIRE(cssMilliSecconds.yearMSB == 0x07);
    REQUIRE(cssMilliSecconds.yearLSB == 0xe4);
    REQUIRE(cssMilliSecconds.month == 2);
    REQUIRE(cssMilliSecconds.day == 29);
    REQUIRE(cssMilliSecconds.hour == 13);
    REQUIRE(cssMilliSecconds.minute == 24);
    REQUIRE(cssMilliSecconds.second == 45);
    uint16_t secondsMinus4 = (static_cast<uint16_t>(cssMilliSecconds.secondEminus2) * 100) +
                             cssMilliSecconds.secondEminus4;
    REQUIRE(secondsMinus4 == 1234);
    Clock::TimeOfDay_t timeTo;
    const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(&cssMilliSecconds);
    size_t length = sizeof(CCSDSTime::Ccs_mseconds);
    result = CCSDSTime::convertFromCCS(&timeTo, dataPtr, &length, sizeof(CCSDSTime::Ccs_mseconds));
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(cssMilliSecconds.pField == 0x52);  // 0b01010010
    REQUIRE(cssMilliSecconds.yearMSB == 0x07);
    REQUIRE(cssMilliSecconds.yearLSB == 0xe4);
    REQUIRE(cssMilliSecconds.month == 2);
    REQUIRE(cssMilliSecconds.day == 29);
    REQUIRE(cssMilliSecconds.hour == 13);
    REQUIRE(cssMilliSecconds.minute == 24);
    REQUIRE(cssMilliSecconds.second == 45);
    REQUIRE(timeTo.year == 2020);
    REQUIRE(timeTo.month == 2);
    REQUIRE(timeTo.day == 29);
    REQUIRE(timeTo.hour == 13);
    REQUIRE(timeTo.minute == 24);
    REQUIRE(timeTo.second == 45);
    REQUIRE(timeTo.usecond == 123400);
  }
  SECTION("CCS_Day of Year"){
    Clock::TimeOfDay_t timeTo;
    std::array<uint8_t,8> ccsDayOfYear = {0b01011000, 0x07, 0xe4, 0, 60, 13, 24, 45};
    size_t length = ccsDayOfYear.size();
    auto result = CCSDSTime::convertFromCCS(&timeTo, ccsDayOfYear.data(), &length, ccsDayOfYear.size());
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    // Check constness
    REQUIRE(ccsDayOfYear[0] == 0b01011000);
    REQUIRE(ccsDayOfYear[1] == 0x07);
    REQUIRE(ccsDayOfYear[2] == 0xe4);
    REQUIRE(ccsDayOfYear[3] == 0x0);
    REQUIRE(ccsDayOfYear[4] == 60);
    REQUIRE(ccsDayOfYear[5] == 13);
    REQUIRE(ccsDayOfYear[6] == 24);
    REQUIRE(ccsDayOfYear[7] == 45);
    REQUIRE(timeTo.year == 2020);
    REQUIRE(timeTo.month == 2);
    REQUIRE(timeTo.day == 29);
    REQUIRE(timeTo.hour == 13);
    REQUIRE(timeTo.minute == 24);
    REQUIRE(timeTo.second == 45);
    REQUIRE(timeTo.usecond == 0);
  }
  SECTION("Test convertFromASCII"){
      std::string timeAscii = "2022-12-31T23:59:59.123Z";
      Clock::TimeOfDay_t timeTo;
      const uint8_t* timeChar = reinterpret_cast<const uint8_t*>(timeAscii.c_str());
      CCSDSTime::convertFromASCII(&timeTo, timeChar, timeAscii.length());
    REQUIRE(timeTo.year == 2022);
    REQUIRE(timeTo.month == 12);
    REQUIRE(timeTo.day == 31);
    REQUIRE(timeTo.hour == 23);
    REQUIRE(timeTo.minute == 59);
    REQUIRE(timeTo.second == 59);
    REQUIRE(timeTo.usecond == Catch::Approx(123000));
  }
}