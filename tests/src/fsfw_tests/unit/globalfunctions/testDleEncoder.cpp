#include "fsfw/globalfunctions/DleEncoder.h"
#include "fsfw_tests/unit/CatchDefinitions.h"
#include "catch2/catch_test_macros.hpp"

#include <array>

const std::array<uint8_t, 5> TEST_ARRAY_0 = { 0 };
const std::array<uint8_t, 3> TEST_ARRAY_1 = { 0, DleEncoder::DLE_CHAR, 5};
const std::array<uint8_t, 3> TEST_ARRAY_2 = { 0, DleEncoder::STX_CHAR, 5};
const std::array<uint8_t, 3> TEST_ARRAY_3 = { 0, DleEncoder::CARRIAGE_RETURN, DleEncoder::ETX_CHAR};

TEST_CASE("DleEncoder" , "[DleEncoder]") {

    DleEncoder dleEncoder;
    std::array<uint8_t, 32> buffer;
    SECTION("Encoding") {
        size_t encodedLen = 0;
        ReturnValue_t result = dleEncoder.encode(TEST_ARRAY_0.data(), TEST_ARRAY_0.size(),
                buffer.data(), buffer.size(), &encodedLen);
        REQUIRE(result == retval::CATCH_OK);
        std::vector<uint8_t> expected = {DleEncoder::STX_CHAR, 0, 0, 0, 0, 0,
                DleEncoder::ETX_CHAR};
        for(size_t idx = 0; idx < expected.size(); idx++) {
            REQUIRE(buffer[idx] == expected[idx]);
        }
        REQUIRE(encodedLen == 7);

        result = dleEncoder.encode(TEST_ARRAY_1.data(), TEST_ARRAY_1.size(),
                buffer.data(), buffer.size(), &encodedLen);
        REQUIRE(result == retval::CATCH_OK);
        expected = std::vector<uint8_t>{DleEncoder::STX_CHAR, 0, DleEncoder::DLE_CHAR,
                DleEncoder::DLE_CHAR, 5, DleEncoder::ETX_CHAR};
        for(size_t idx = 0; idx < expected.size(); idx++) {
            REQUIRE(buffer[idx] == expected[idx]);
        }
        REQUIRE(encodedLen == expected.size());

        result = dleEncoder.encode(TEST_ARRAY_2.data(), TEST_ARRAY_2.size(),
                buffer.data(), buffer.size(), &encodedLen);
        REQUIRE(result == retval::CATCH_OK);
        expected = std::vector<uint8_t>{DleEncoder::STX_CHAR, 0, DleEncoder::DLE_CHAR,
               DleEncoder::STX_CHAR + 0x40, 5, DleEncoder::ETX_CHAR};
        for(size_t idx = 0; idx < expected.size(); idx++) {
            REQUIRE(buffer[idx] == expected[idx]);
        }
        REQUIRE(encodedLen == expected.size());

        result = dleEncoder.encode(TEST_ARRAY_3.data(), TEST_ARRAY_3.size(),
                buffer.data(), buffer.size(), &encodedLen);
        REQUIRE(result == retval::CATCH_OK);
        expected = std::vector<uint8_t>{DleEncoder::STX_CHAR, 0, DleEncoder::CARRIAGE_RETURN,
            DleEncoder::DLE_CHAR, DleEncoder::ETX_CHAR + 0x40, DleEncoder::ETX_CHAR};
        for(size_t idx = 0; idx < expected.size(); idx++) {
            REQUIRE(buffer[idx] == expected[idx]);
        }
        REQUIRE(encodedLen == expected.size());

        result = dleEncoder.encode(TEST_ARRAY_3.data(), TEST_ARRAY_3.size(),
                buffer.data(), 0, &encodedLen);
        REQUIRE(result == DleEncoder::STREAM_TOO_SHORT);
        result = dleEncoder.encode(TEST_ARRAY_1.data(), TEST_ARRAY_1.size(),
                buffer.data(), 4, &encodedLen);
        REQUIRE(result == DleEncoder::STREAM_TOO_SHORT);
    }

    SECTION("Decoding") {

    }
}
