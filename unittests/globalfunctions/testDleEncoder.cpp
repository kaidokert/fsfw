#include <array>

#include "CatchDefinitions.h"
#include "catch2/catch_test_macros.hpp"
#include "fsfw/globalfunctions/DleEncoder.h"

const std::vector<uint8_t> TEST_ARRAY_0 = {0, 0, 0, 0, 0};
const std::vector<uint8_t> TEST_ARRAY_1 = {0, DleEncoder::DLE_CHAR, 5};
const std::vector<uint8_t> TEST_ARRAY_2 = {0, DleEncoder::STX_CHAR, 5};
const std::vector<uint8_t> TEST_ARRAY_3 = {0, DleEncoder::CARRIAGE_RETURN, DleEncoder::ETX_CHAR};
const std::vector<uint8_t> TEST_ARRAY_4 = {DleEncoder::DLE_CHAR, DleEncoder::ETX_CHAR,
                                           DleEncoder::STX_CHAR};

const std::vector<uint8_t> TEST_ARRAY_0_ENCODED_ESCAPED = {DleEncoder::STX_CHAR, 0, 0, 0, 0, 0,
                                                           DleEncoder::ETX_CHAR};
const std::vector<uint8_t> TEST_ARRAY_0_ENCODED_NON_ESCAPED = {
    DleEncoder::DLE_CHAR, DleEncoder::STX_CHAR, 0, 0, 0, 0, 0,
    DleEncoder::DLE_CHAR, DleEncoder::ETX_CHAR};

const std::vector<uint8_t> TEST_ARRAY_1_ENCODED_ESCAPED = {
    DleEncoder::STX_CHAR, 0, DleEncoder::DLE_CHAR, DleEncoder::DLE_CHAR, 5, DleEncoder::ETX_CHAR};
const std::vector<uint8_t> TEST_ARRAY_1_ENCODED_NON_ESCAPED = {
    DleEncoder::DLE_CHAR, DleEncoder::STX_CHAR, 0, DleEncoder::DLE_CHAR, DleEncoder::DLE_CHAR, 5,
    DleEncoder::DLE_CHAR, DleEncoder::ETX_CHAR};

const std::vector<uint8_t> TEST_ARRAY_2_ENCODED_ESCAPED = {
    DleEncoder::STX_CHAR,        0, DleEncoder::DLE_CHAR,
    DleEncoder::STX_CHAR + 0x40, 5, DleEncoder::ETX_CHAR};
const std::vector<uint8_t> TEST_ARRAY_2_ENCODED_NON_ESCAPED = {
    DleEncoder::DLE_CHAR, DleEncoder::STX_CHAR, 0, DleEncoder::STX_CHAR, 5,
    DleEncoder::DLE_CHAR, DleEncoder::ETX_CHAR};

const std::vector<uint8_t> TEST_ARRAY_3_ENCODED_ESCAPED = {
    DleEncoder::STX_CHAR,        0,
    DleEncoder::CARRIAGE_RETURN, DleEncoder::DLE_CHAR,
    DleEncoder::ETX_CHAR + 0x40, DleEncoder::ETX_CHAR};
const std::vector<uint8_t> TEST_ARRAY_3_ENCODED_NON_ESCAPED = {
    DleEncoder::DLE_CHAR,        DleEncoder::STX_CHAR, 0,
    DleEncoder::CARRIAGE_RETURN, DleEncoder::ETX_CHAR, DleEncoder::DLE_CHAR,
    DleEncoder::ETX_CHAR};

const std::vector<uint8_t> TEST_ARRAY_4_ENCODED_ESCAPED = {
    DleEncoder::STX_CHAR,        DleEncoder::DLE_CHAR,        DleEncoder::DLE_CHAR,
    DleEncoder::DLE_CHAR,        DleEncoder::ETX_CHAR + 0x40, DleEncoder::DLE_CHAR,
    DleEncoder::STX_CHAR + 0x40, DleEncoder::ETX_CHAR};
const std::vector<uint8_t> TEST_ARRAY_4_ENCODED_NON_ESCAPED = {
    DleEncoder::DLE_CHAR, DleEncoder::STX_CHAR, DleEncoder::DLE_CHAR, DleEncoder::DLE_CHAR,
    DleEncoder::ETX_CHAR, DleEncoder::STX_CHAR, DleEncoder::DLE_CHAR, DleEncoder::ETX_CHAR};

TEST_CASE("DleEncoder", "[DleEncoder]") {
  DleEncoder dleEncoder;
  ReturnValue_t result = returnvalue::OK;
  std::array<uint8_t, 32> buffer;

  size_t encodedLen = 0;
  size_t readLen = 0;
  size_t decodedLen = 0;

  auto testLambdaEncode = [&](DleEncoder& encoder, const std::vector<uint8_t>& vecToEncode,
                              const std::vector<uint8_t>& expectedVec) {
    result = encoder.encode(vecToEncode.data(), vecToEncode.size(), buffer.data(), buffer.size(),
                            &encodedLen);
    REQUIRE(result == returnvalue::OK);
    for (size_t idx = 0; idx < expectedVec.size(); idx++) {
      REQUIRE(buffer[idx] == expectedVec[idx]);
    }
    REQUIRE(encodedLen == expectedVec.size());
  };

  auto testLambdaDecode = [&](DleEncoder& encoder, const std::vector<uint8_t>& testVecEncoded,
                              const std::vector<uint8_t>& expectedVec) {
    result = encoder.decode(testVecEncoded.data(), testVecEncoded.size(), &readLen, buffer.data(),
                            buffer.size(), &decodedLen);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(readLen == testVecEncoded.size());
    REQUIRE(decodedLen == expectedVec.size());
    for (size_t idx = 0; idx < decodedLen; idx++) {
      REQUIRE(buffer[idx] == expectedVec[idx]);
    }
  };

  SECTION("Encoding") {
    testLambdaEncode(dleEncoder, TEST_ARRAY_0, TEST_ARRAY_0_ENCODED_ESCAPED);
    testLambdaEncode(dleEncoder, TEST_ARRAY_1, TEST_ARRAY_1_ENCODED_ESCAPED);
    testLambdaEncode(dleEncoder, TEST_ARRAY_2, TEST_ARRAY_2_ENCODED_ESCAPED);
    testLambdaEncode(dleEncoder, TEST_ARRAY_3, TEST_ARRAY_3_ENCODED_ESCAPED);
    testLambdaEncode(dleEncoder, TEST_ARRAY_4, TEST_ARRAY_4_ENCODED_ESCAPED);

    auto testFaultyEncoding = [&](const std::vector<uint8_t>& vecToEncode,
                                  const std::vector<uint8_t>& expectedVec) {
      for (size_t faultyDestSize = 0; faultyDestSize < expectedVec.size(); faultyDestSize++) {
        result = dleEncoder.encode(vecToEncode.data(), vecToEncode.size(), buffer.data(),
                                   faultyDestSize, &encodedLen);
        REQUIRE(result == static_cast<int>(DleEncoder::STREAM_TOO_SHORT));
      }
    };

    testFaultyEncoding(TEST_ARRAY_0, TEST_ARRAY_0_ENCODED_ESCAPED);
    testFaultyEncoding(TEST_ARRAY_1, TEST_ARRAY_1_ENCODED_ESCAPED);
    testFaultyEncoding(TEST_ARRAY_2, TEST_ARRAY_2_ENCODED_ESCAPED);
    testFaultyEncoding(TEST_ARRAY_3, TEST_ARRAY_3_ENCODED_ESCAPED);
    testFaultyEncoding(TEST_ARRAY_4, TEST_ARRAY_4_ENCODED_ESCAPED);

    dleEncoder.setEscapeMode(false);
    testLambdaEncode(dleEncoder, TEST_ARRAY_0, TEST_ARRAY_0_ENCODED_NON_ESCAPED);
    testLambdaEncode(dleEncoder, TEST_ARRAY_1, TEST_ARRAY_1_ENCODED_NON_ESCAPED);
    testLambdaEncode(dleEncoder, TEST_ARRAY_2, TEST_ARRAY_2_ENCODED_NON_ESCAPED);
    testLambdaEncode(dleEncoder, TEST_ARRAY_3, TEST_ARRAY_3_ENCODED_NON_ESCAPED);
    testLambdaEncode(dleEncoder, TEST_ARRAY_4, TEST_ARRAY_4_ENCODED_NON_ESCAPED);

    testFaultyEncoding(TEST_ARRAY_0, TEST_ARRAY_0_ENCODED_NON_ESCAPED);
    testFaultyEncoding(TEST_ARRAY_1, TEST_ARRAY_1_ENCODED_NON_ESCAPED);
    testFaultyEncoding(TEST_ARRAY_2, TEST_ARRAY_2_ENCODED_NON_ESCAPED);
    testFaultyEncoding(TEST_ARRAY_3, TEST_ARRAY_3_ENCODED_NON_ESCAPED);
    testFaultyEncoding(TEST_ARRAY_4, TEST_ARRAY_4_ENCODED_NON_ESCAPED);
    dleEncoder.setEscapeMode(true);
  }

  SECTION("Decoding") {
    testLambdaDecode(dleEncoder, TEST_ARRAY_0_ENCODED_ESCAPED, TEST_ARRAY_0);
    testLambdaDecode(dleEncoder, TEST_ARRAY_1_ENCODED_ESCAPED, TEST_ARRAY_1);
    testLambdaDecode(dleEncoder, TEST_ARRAY_2_ENCODED_ESCAPED, TEST_ARRAY_2);
    testLambdaDecode(dleEncoder, TEST_ARRAY_3_ENCODED_ESCAPED, TEST_ARRAY_3);
    testLambdaDecode(dleEncoder, TEST_ARRAY_4_ENCODED_ESCAPED, TEST_ARRAY_4);

    // Faulty source data
    auto testArray1EncodedFaulty = TEST_ARRAY_1_ENCODED_ESCAPED;
    testArray1EncodedFaulty[3] = 0;
    result = dleEncoder.decode(testArray1EncodedFaulty.data(), testArray1EncodedFaulty.size(),
                               &readLen, buffer.data(), buffer.size(), &encodedLen);
    REQUIRE(result == static_cast<int>(DleEncoder::DECODING_ERROR));
    auto testArray2EncodedFaulty = TEST_ARRAY_2_ENCODED_ESCAPED;
    testArray2EncodedFaulty[5] = 0;
    result = dleEncoder.decode(testArray2EncodedFaulty.data(), testArray2EncodedFaulty.size(),
                               &readLen, buffer.data(), buffer.size(), &encodedLen);
    REQUIRE(result == static_cast<int>(DleEncoder::DECODING_ERROR));
    auto testArray4EncodedFaulty = TEST_ARRAY_4_ENCODED_ESCAPED;
    testArray4EncodedFaulty[2] = 0;
    result = dleEncoder.decode(testArray4EncodedFaulty.data(), testArray4EncodedFaulty.size(),
                               &readLen, buffer.data(), buffer.size(), &encodedLen);
    REQUIRE(result == static_cast<int>(DleEncoder::DECODING_ERROR));
    auto testArray4EncodedFaulty2 = TEST_ARRAY_4_ENCODED_ESCAPED;
    testArray4EncodedFaulty2[4] = 0;
    result = dleEncoder.decode(testArray4EncodedFaulty2.data(), testArray4EncodedFaulty2.size(),
                               &readLen, buffer.data(), buffer.size(), &encodedLen);
    REQUIRE(result == static_cast<int>(DleEncoder::DECODING_ERROR));

    auto testFaultyDecoding = [&](const std::vector<uint8_t>& vecToDecode,
                                  const std::vector<uint8_t>& expectedVec) {
      for (size_t faultyDestSizes = 0; faultyDestSizes < expectedVec.size(); faultyDestSizes++) {
        result = dleEncoder.decode(vecToDecode.data(), vecToDecode.size(), &readLen, buffer.data(),
                                   faultyDestSizes, &decodedLen);
        REQUIRE(result == static_cast<int>(DleEncoder::STREAM_TOO_SHORT));
      }
    };

    testFaultyDecoding(TEST_ARRAY_0_ENCODED_ESCAPED, TEST_ARRAY_0);
    testFaultyDecoding(TEST_ARRAY_1_ENCODED_ESCAPED, TEST_ARRAY_1);
    testFaultyDecoding(TEST_ARRAY_2_ENCODED_ESCAPED, TEST_ARRAY_2);
    testFaultyDecoding(TEST_ARRAY_3_ENCODED_ESCAPED, TEST_ARRAY_3);
    testFaultyDecoding(TEST_ARRAY_4_ENCODED_ESCAPED, TEST_ARRAY_4);

    dleEncoder.setEscapeMode(false);
    testLambdaDecode(dleEncoder, TEST_ARRAY_0_ENCODED_NON_ESCAPED, TEST_ARRAY_0);
    testLambdaDecode(dleEncoder, TEST_ARRAY_1_ENCODED_NON_ESCAPED, TEST_ARRAY_1);
    testLambdaDecode(dleEncoder, TEST_ARRAY_2_ENCODED_NON_ESCAPED, TEST_ARRAY_2);
    testLambdaDecode(dleEncoder, TEST_ARRAY_3_ENCODED_NON_ESCAPED, TEST_ARRAY_3);
    testLambdaDecode(dleEncoder, TEST_ARRAY_4_ENCODED_NON_ESCAPED, TEST_ARRAY_4);

    testFaultyDecoding(TEST_ARRAY_0_ENCODED_NON_ESCAPED, TEST_ARRAY_0);
    testFaultyDecoding(TEST_ARRAY_1_ENCODED_NON_ESCAPED, TEST_ARRAY_1);
    testFaultyDecoding(TEST_ARRAY_2_ENCODED_NON_ESCAPED, TEST_ARRAY_2);
    testFaultyDecoding(TEST_ARRAY_3_ENCODED_NON_ESCAPED, TEST_ARRAY_3);
    testFaultyDecoding(TEST_ARRAY_4_ENCODED_NON_ESCAPED, TEST_ARRAY_4);

    // Faulty source data
    testArray1EncodedFaulty = TEST_ARRAY_1_ENCODED_NON_ESCAPED;
    auto prevVal = testArray1EncodedFaulty[0];
    testArray1EncodedFaulty[0] = 0;
    result = dleEncoder.decode(testArray1EncodedFaulty.data(), testArray1EncodedFaulty.size(),
                               &readLen, buffer.data(), buffer.size(), &encodedLen);
    REQUIRE(result == static_cast<int>(DleEncoder::DECODING_ERROR));
    testArray1EncodedFaulty[0] = prevVal;
    testArray1EncodedFaulty[1] = 0;
    result = dleEncoder.decode(testArray1EncodedFaulty.data(), testArray1EncodedFaulty.size(),
                               &readLen, buffer.data(), buffer.size(), &encodedLen);
    REQUIRE(result == static_cast<int>(DleEncoder::DECODING_ERROR));

    testArray1EncodedFaulty = TEST_ARRAY_1_ENCODED_NON_ESCAPED;
    testArray1EncodedFaulty[6] = 0;
    result = dleEncoder.decode(testArray1EncodedFaulty.data(), testArray1EncodedFaulty.size(),
                               &readLen, buffer.data(), buffer.size(), &encodedLen);
    REQUIRE(result == static_cast<int>(DleEncoder::DECODING_ERROR));
    testArray1EncodedFaulty = TEST_ARRAY_1_ENCODED_NON_ESCAPED;
    testArray1EncodedFaulty[7] = 0;
    result = dleEncoder.decode(testArray1EncodedFaulty.data(), testArray1EncodedFaulty.size(),
                               &readLen, buffer.data(), buffer.size(), &encodedLen);
    REQUIRE(result == static_cast<int>(DleEncoder::DECODING_ERROR));
    testArray4EncodedFaulty = TEST_ARRAY_4_ENCODED_NON_ESCAPED;
    testArray4EncodedFaulty[3] = 0;
    result = dleEncoder.decode(testArray4EncodedFaulty.data(), testArray4EncodedFaulty.size(),
                               &readLen, buffer.data(), buffer.size(), &encodedLen);
    REQUIRE(result == static_cast<int>(DleEncoder::DECODING_ERROR));

    dleEncoder.setEscapeMode(true);
    testArray1EncodedFaulty = TEST_ARRAY_1_ENCODED_ESCAPED;
    testArray1EncodedFaulty[5] = 0;
    result = dleEncoder.decode(testArray1EncodedFaulty.data(), testArray1EncodedFaulty.size(),
                               &readLen, buffer.data(), buffer.size(), &encodedLen);
    REQUIRE(result == static_cast<int>(DleEncoder::DECODING_ERROR));
  }
}
