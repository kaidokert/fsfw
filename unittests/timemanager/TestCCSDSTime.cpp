#include <fsfw/globalfunctions/timevalOperations.h>
#include <fsfw/timemanager/CCSDSTime.h>

#include <array>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"

TEST_CASE("CCSDSTime Tests", "[TestCCSDSTime]") {
  INFO("CCSDSTime Tests");
  CCSDSTime::Ccs_mseconds cssMilliSecconds{};
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
    REQUIRE(result == returnvalue::OK);
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
    REQUIRE(result == returnvalue::OK);
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
  SECTION("CCS_Day of Year") {
    Clock::TimeOfDay_t timeTo;
    std::array<uint8_t, 8> ccsDayOfYear = {0b01011000, 0x07, 0xe4, 0, 60, 13, 24, 45};
    size_t length = ccsDayOfYear.size();
    auto result =
        CCSDSTime::convertFromCCS(&timeTo, ccsDayOfYear.data(), &length, ccsDayOfYear.size());
    REQUIRE(result == returnvalue::OK);
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
  SECTION("Test convertFromASCII") {
    std::string timeAscii = "2022-12-31T23:59:59.123Z";
    Clock::TimeOfDay_t timeTo;
    const uint8_t* timeChar = reinterpret_cast<const uint8_t*>(timeAscii.c_str());
    auto result = CCSDSTime::convertFromASCII(&timeTo, timeChar, timeAscii.length());
    REQUIRE(result == returnvalue::OK);
    REQUIRE(timeTo.year == 2022);
    REQUIRE(timeTo.month == 12);
    REQUIRE(timeTo.day == 31);
    REQUIRE(timeTo.hour == 23);
    REQUIRE(timeTo.minute == 59);
    REQUIRE(timeTo.second == 59);
    REQUIRE(timeTo.usecond == Catch::Approx(123000));

    std::string timeAscii2 = "2022-365T23:59:59.123Z";
    const uint8_t* timeChar2 = reinterpret_cast<const uint8_t*>(timeAscii2.c_str());
    Clock::TimeOfDay_t timeTo2;
    result = CCSDSTime::convertFromCcsds(&timeTo2, timeChar2, timeAscii2.length());
    REQUIRE(result == returnvalue::OK);
    REQUIRE(timeTo2.year == 2022);
    REQUIRE(timeTo2.month == 12);
    REQUIRE(timeTo2.day == 31);
    REQUIRE(timeTo2.hour == 23);
    REQUIRE(timeTo2.minute == 59);
    REQUIRE(timeTo2.second == 59);
    REQUIRE(timeTo2.usecond == Catch::Approx(123000));
  }

  SECTION("CDS Conversions") {
    // Preperation
    Clock::TimeOfDay_t time;
    time.year = 2020;
    time.month = 2;
    time.day = 29;
    time.hour = 13;
    time.minute = 24;
    time.second = 45;
    time.usecond = 123456;
    timeval timeAsTimeval;
    auto result = Clock::convertTimeOfDayToTimeval(&time, &timeAsTimeval);
    CHECK(result == returnvalue::OK);
    CHECK(timeAsTimeval.tv_sec == 1582982685);
    CHECK(timeAsTimeval.tv_usec == 123456);

    // Conversion to CDS Short
    CCSDSTime::CDS_short cdsTime;
    result = CCSDSTime::convertToCcsds(&cdsTime, &timeAsTimeval);
    CHECK(result == returnvalue::OK);
    // Days in CCSDS Epoch 22704 (0x58B0)
    CHECK(cdsTime.dayMSB == 0x58);
    CHECK(cdsTime.dayLSB == 0xB0);
    // MS of day 48285123.456 (floored here)
    CHECK(cdsTime.msDay_hh == 0x2);
    CHECK(cdsTime.msDay_h == 0xE0);
    CHECK(cdsTime.msDay_l == 0xC5);
    CHECK(cdsTime.msDay_ll == 0xC3);
    CHECK(cdsTime.pField == CCSDSTime::P_FIELD_CDS_SHORT);

    // Conversion back to timeval
    timeval timeReturnAsTimeval;
    result = CCSDSTime::convertFromCDS(&timeReturnAsTimeval, &cdsTime);
    CHECK(result == returnvalue::OK);
    // micro seconds precision is lost
    timeval difference = timeAsTimeval - timeReturnAsTimeval;
    CHECK(difference.tv_usec == 456);
    CHECK(difference.tv_sec == 0);

    Clock::TimeOfDay_t timeReturnAsTimeOfDay;
    result = CCSDSTime::convertFromCDS(&timeReturnAsTimeOfDay, &cdsTime);
    CHECK(result == returnvalue::OK);
    CHECK(timeReturnAsTimeOfDay.year == 2020);
    CHECK(timeReturnAsTimeOfDay.month == 2);
    CHECK(timeReturnAsTimeOfDay.day == 29);
    CHECK(timeReturnAsTimeOfDay.hour == 13);
    CHECK(timeReturnAsTimeOfDay.minute == 24);
    CHECK(timeReturnAsTimeOfDay.second == 45);
    // micro seconds precision is lost
    CHECK(timeReturnAsTimeOfDay.usecond == 123000);

    Clock::TimeOfDay_t timeReturnAsTodFromBuffer;
    const uint8_t* buffer = reinterpret_cast<const uint8_t*>(&cdsTime);
    result = CCSDSTime::convertFromCDS(&timeReturnAsTodFromBuffer, buffer, sizeof(cdsTime));
    REQUIRE(result == returnvalue::OK);
    CHECK(timeReturnAsTodFromBuffer.year == time.year);
    CHECK(timeReturnAsTodFromBuffer.month == time.month);
    CHECK(timeReturnAsTodFromBuffer.day == time.day);
    CHECK(timeReturnAsTodFromBuffer.hour == time.hour);
    CHECK(timeReturnAsTodFromBuffer.minute == time.minute);
    CHECK(timeReturnAsTodFromBuffer.second == time.second);
    CHECK(timeReturnAsTodFromBuffer.usecond == 123000);

    Clock::TimeOfDay_t todFromCCSDS;
    result = CCSDSTime::convertFromCcsds(&todFromCCSDS, buffer, sizeof(cdsTime));
    CHECK(result == returnvalue::OK);
    CHECK(todFromCCSDS.year == time.year);
    CHECK(todFromCCSDS.month == time.month);
    CHECK(todFromCCSDS.day == time.day);
    CHECK(todFromCCSDS.hour == time.hour);
    CHECK(todFromCCSDS.minute == time.minute);
    CHECK(todFromCCSDS.second == time.second);
    CHECK(todFromCCSDS.usecond == 123000);
  }
  SECTION("CUC") {
    timeval to;
    // seconds = 0x771E960F, microseconds = 0x237
    // microseconds = 567000
    // This gives 37158.912 1/65536 seconds -> rounded to 37159 -> 0x9127
    // This results in -> 567001 us
    std::array<uint8_t, 7> cucBuffer = {
        CCSDSTime::P_FIELD_CUC_6B_CCSDS, 0x77, 0x1E, 0x96, 0x0F, 0x91, 0x27};
    size_t foundLength = 0;
    auto result = CCSDSTime::convertFromCUC(&to, cucBuffer.data(), &foundLength, cucBuffer.size());
    REQUIRE(result == returnvalue::OK);
    REQUIRE(foundLength == 7);
    REQUIRE(to.tv_sec == 1619801999);  // TAI (no leap seconds)
    REQUIRE(to.tv_usec == 567001);

    Clock::TimeOfDay_t tod;
    result = CCSDSTime::convertFromCUC(&tod, cucBuffer.data(), cucBuffer.size());
    // This test must be changed if this is ever going to be implemented
    REQUIRE(result == CCSDSTime::UNSUPPORTED_TIME_FORMAT);
  }

  SECTION("CCSDS Failures") {
    Clock::TimeOfDay_t time;
    time.year = 2020;
    time.month = 12;
    time.day = 32;
    time.hour = 13;
    time.minute = 24;
    time.second = 45;
    time.usecond = 123456;
    CCSDSTime::Ccs_mseconds to;
    auto result = CCSDSTime::convertToCcsds(&to, &time);
    REQUIRE(result == CCSDSTime::INVALID_TIME_FORMAT);
    CCSDSTime::Ccs_seconds to2;
    result = CCSDSTime::convertToCcsds(&to2, &time);
    REQUIRE(result == CCSDSTime::INVALID_TIME_FORMAT);
  }
}