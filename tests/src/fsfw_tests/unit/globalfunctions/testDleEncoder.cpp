#include "fsfw/globalfunctions/DleEncoder.h"
#include "fsfw_tests/unit/CatchDefinitions.h"
#include "catch2/catch_test_macros.hpp"

#include <array>

const std::vector<uint8_t> TEST_ARRAY_0 = { 0, 0, 0, 0, 0 };
const std::vector<uint8_t> TEST_ARRAY_1 = { 0, DleEncoder::DLE_CHAR, 5};
const std::vector<uint8_t> TEST_ARRAY_2 = { 0, DleEncoder::STX_CHAR, 5};
const std::vector<uint8_t> TEST_ARRAY_3 = { 0, DleEncoder::CARRIAGE_RETURN, DleEncoder::ETX_CHAR};
const std::vector<uint8_t> TEST_ARRAY_4 = { DleEncoder::DLE_CHAR, DleEncoder::ETX_CHAR,
        DleEncoder::STX_CHAR };

const std::vector<uint8_t> TEST_ARRAY_0_ENCODED_ESCAPED = {
        DleEncoder::STX_CHAR, 0, 0, 0, 0, 0, DleEncoder::ETX_CHAR
};
const std::vector<uint8_t> TEST_ARRAY_0_ENCODED_NON_ESCAPED = {
        DleEncoder::DLE_CHAR, DleEncoder::STX_CHAR, 0, 0, 0, 0, 0,
        DleEncoder::DLE_CHAR, DleEncoder::ETX_CHAR
};

const std::vector<uint8_t> TEST_ARRAY_1_ENCODED_ESCAPED = {
        DleEncoder::STX_CHAR, 0, DleEncoder::DLE_CHAR, DleEncoder::DLE_CHAR, 5, DleEncoder::ETX_CHAR
};
const std::vector<uint8_t> TEST_ARRAY_2_ENCODED_ESCAPED = {
        DleEncoder::STX_CHAR, 0, DleEncoder::DLE_CHAR, DleEncoder::STX_CHAR + 0x40,
        5, DleEncoder::ETX_CHAR
};
const std::vector<uint8_t> TEST_ARRAY_3_ENCODED_ESCAPED = {
        DleEncoder::STX_CHAR, 0, DleEncoder::CARRIAGE_RETURN,
        DleEncoder::DLE_CHAR, DleEncoder::ETX_CHAR + 0x40, DleEncoder::ETX_CHAR
};
const std::vector<uint8_t> TEST_ARRAY_4_ENCODED_ESCAPED = {
        DleEncoder::STX_CHAR, DleEncoder::DLE_CHAR, DleEncoder::DLE_CHAR,
        DleEncoder::DLE_CHAR, DleEncoder::ETX_CHAR + 0x40, DleEncoder::DLE_CHAR,
        DleEncoder::STX_CHAR + 0x40, DleEncoder::ETX_CHAR
};


TEST_CASE("DleEncoder" , "[DleEncoder]") {
    DleEncoder dleEncoder;
    std::array<uint8_t, 32> buffer;

    size_t encodedLen = 0;
    size_t readLen = 0;
    size_t decodedLen = 0;

    auto testLambdaEncode = [&](DleEncoder& encoder, const std::vector<uint8_t>& vecToEncode,
            const std::vector<uint8_t>& expectedVec) {
        ReturnValue_t result = encoder.encode(vecToEncode.data(), vecToEncode.size(),
                buffer.data(), buffer.size(), &encodedLen);
        REQUIRE(result == retval::CATCH_OK);
        for(size_t idx = 0; idx < expectedVec.size(); idx++) {
            REQUIRE(buffer[idx] == expectedVec[idx]);
        }
        REQUIRE(encodedLen == expectedVec.size());
    };

    auto testLambdaDecode = [&](DleEncoder& encoder, const std::vector<uint8_t>& testVecEncoded,
            const std::vector<uint8_t>& expectedVec) {
        ReturnValue_t result = encoder.decode(testVecEncoded.data(),
                testVecEncoded.size(),
                &readLen, buffer.data(), buffer.size(), &decodedLen);
        REQUIRE(result == retval::CATCH_OK);
        REQUIRE(readLen == testVecEncoded.size());
        REQUIRE(decodedLen == expectedVec.size());
        for(size_t idx = 0; idx < decodedLen; idx++) {
            REQUIRE(buffer[idx] == expectedVec[idx]);
        }
    };

    SECTION("Encoding") {
        testLambdaEncode(dleEncoder, TEST_ARRAY_0, TEST_ARRAY_0_ENCODED_ESCAPED);
        testLambdaEncode(dleEncoder, TEST_ARRAY_1, TEST_ARRAY_1_ENCODED_ESCAPED);
        testLambdaEncode(dleEncoder, TEST_ARRAY_2, TEST_ARRAY_2_ENCODED_ESCAPED);
        testLambdaEncode(dleEncoder, TEST_ARRAY_3, TEST_ARRAY_3_ENCODED_ESCAPED);
        testLambdaEncode(dleEncoder, TEST_ARRAY_4, TEST_ARRAY_4_ENCODED_ESCAPED);

        dleEncoder.setEscapeMode(false);
        testLambdaEncode(dleEncoder, TEST_ARRAY_0, TEST_ARRAY_0_ENCODED_NON_ESCAPED);

        ReturnValue_t result = dleEncoder.encode(TEST_ARRAY_3.data(), TEST_ARRAY_3.size(),
                buffer.data(), 0, &encodedLen);
        REQUIRE(result == DleEncoder::STREAM_TOO_SHORT);
        result = dleEncoder.encode(TEST_ARRAY_1.data(), TEST_ARRAY_1.size(),
                buffer.data(), 4, &encodedLen);
        REQUIRE(result == DleEncoder::STREAM_TOO_SHORT);
    }

    SECTION("Decoding") {
        testLambdaDecode(dleEncoder, TEST_ARRAY_0_ENCODED_ESCAPED, TEST_ARRAY_0);
        testLambdaDecode(dleEncoder, TEST_ARRAY_1_ENCODED_ESCAPED, TEST_ARRAY_1);
        testLambdaDecode(dleEncoder, TEST_ARRAY_2_ENCODED_ESCAPED, TEST_ARRAY_2);
        testLambdaDecode(dleEncoder, TEST_ARRAY_3_ENCODED_ESCAPED, TEST_ARRAY_3);
        testLambdaDecode(dleEncoder, TEST_ARRAY_4_ENCODED_ESCAPED, TEST_ARRAY_4);
    }
}
