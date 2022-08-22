#include <fsfw/container/SimpleRingBuffer.h>

#include <catch2/catch_test_macros.hpp>
#include <cstring>

#include "CatchDefinitions.h"

TEST_CASE("Ring Buffer Test", "[containers]") {
  uint8_t testData[13] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  uint8_t readBuffer[10] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13};
  SimpleRingBuffer ringBuffer(10, false, 5);

  SECTION("Simple Test") {
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 9) == returnvalue::OK);
    REQUIRE(ringBuffer.writeData(testData, 3) == returnvalue::FAILED);
    REQUIRE(ringBuffer.readData(readBuffer, 5, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 5; i++) {
      CHECK(readBuffer[i] == i);
    }
    REQUIRE(ringBuffer.availableWriteSpace() == 5);
    ringBuffer.clear();
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 4) == returnvalue::OK);
    REQUIRE(ringBuffer.readData(readBuffer, 4, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 4; i++) {
      CHECK(readBuffer[i] == i);
    }
    REQUIRE(ringBuffer.writeData(testData, 9) == returnvalue::OK);
    REQUIRE(ringBuffer.readData(readBuffer, 9, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 9; i++) {
      CHECK(readBuffer[i] == i);
    }
    REQUIRE(ringBuffer.writeData(testData, 1024) == returnvalue::FAILED);
    REQUIRE(ringBuffer.writeData(nullptr, 5) == returnvalue::FAILED);
  }

  SECTION("Get Free Element Test") {
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 8) == returnvalue::OK);
    REQUIRE(ringBuffer.availableWriteSpace() == 1);
    REQUIRE(ringBuffer.readData(readBuffer, 8, true) == returnvalue::OK);
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    uint8_t *testPtr = nullptr;
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 10) == returnvalue::FAILED);

    REQUIRE(ringBuffer.writeTillWrap() == 2);
    // too many excess bytes.
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 8) == returnvalue::FAILED);
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 5) == returnvalue::OK);
    REQUIRE(ringBuffer.getExcessBytes() == 3);
    std::memcpy(testPtr, testData, 5);
    ringBuffer.confirmBytesWritten(5);
    REQUIRE(ringBuffer.getAvailableReadData() == 5);
    ringBuffer.readData(readBuffer, 5, true);
    for (uint8_t i = 0; i < 5; i++) {
      CHECK(readBuffer[i] == i);
    }
  }

  SECTION("Read Remaining Test") {
    REQUIRE(ringBuffer.writeData(testData, 3) == returnvalue::OK);
    REQUIRE(ringBuffer.getAvailableReadData() == 3);
    REQUIRE(ringBuffer.readData(readBuffer, 5, false, false, nullptr) == returnvalue::FAILED);
    size_t trueSize = 0;
    REQUIRE(ringBuffer.readData(readBuffer, 5, false, true, &trueSize) == returnvalue::OK);
    REQUIRE(trueSize == 3);
    for (uint8_t i = 0; i < 3; i++) {
      CHECK(readBuffer[i] == i);
    }
    trueSize = 0;
    REQUIRE(ringBuffer.deleteData(5, false, &trueSize) == returnvalue::FAILED);
    REQUIRE(trueSize == 0);
    REQUIRE(ringBuffer.deleteData(5, true, &trueSize) == returnvalue::OK);
    REQUIRE(trueSize == 3);
  }
}

TEST_CASE("Ring Buffer Test2", "[RingBufferTest2]") {
  uint8_t testData[13] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  uint8_t readBuffer[10] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13};
  uint8_t *newBuffer = new uint8_t[15];
  SimpleRingBuffer ringBuffer(newBuffer, 10, true, 5);

  SECTION("Simple Test") {
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 9) == returnvalue::OK);
    REQUIRE(ringBuffer.readData(readBuffer, 5, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 5; i++) {
      CHECK(readBuffer[i] == i);
    }
    REQUIRE(ringBuffer.availableWriteSpace() == 5);
    ringBuffer.clear();
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 4) == returnvalue::OK);
    REQUIRE(ringBuffer.readData(readBuffer, 4, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 4; i++) {
      CHECK(readBuffer[i] == i);
    }
    REQUIRE(ringBuffer.writeData(testData, 9) == returnvalue::OK);
    REQUIRE(ringBuffer.readData(readBuffer, 9, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 9; i++) {
      CHECK(readBuffer[i] == i);
    }
  }

  SECTION("Get Free Element Test") {
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 8) == returnvalue::OK);
    REQUIRE(ringBuffer.availableWriteSpace() == 1);
    REQUIRE(ringBuffer.readData(readBuffer, 8, true) == returnvalue::OK);
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    uint8_t *testPtr = nullptr;
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 10) == returnvalue::FAILED);

    REQUIRE(ringBuffer.writeTillWrap() == 2);
    // too many excess bytes.
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 8) == returnvalue::FAILED);
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 5) == returnvalue::OK);
    REQUIRE(ringBuffer.getExcessBytes() == 3);
    std::memcpy(testPtr, testData, 5);
    ringBuffer.confirmBytesWritten(5);
    REQUIRE(ringBuffer.getAvailableReadData() == 5);
    ringBuffer.readData(readBuffer, 5, true);
    for (uint8_t i = 0; i < 5; i++) {
      CHECK(readBuffer[i] == i);
    }
  }

  SECTION("Read Remaining Test") {
    REQUIRE(ringBuffer.writeData(testData, 3) == returnvalue::OK);
    REQUIRE(ringBuffer.getAvailableReadData() == 3);
    REQUIRE(ringBuffer.readData(readBuffer, 5, false, false, nullptr) == returnvalue::FAILED);
    size_t trueSize = 0;
    REQUIRE(ringBuffer.readData(readBuffer, 5, false, true, &trueSize) == returnvalue::OK);
    REQUIRE(trueSize == 3);
    for (uint8_t i = 0; i < 3; i++) {
      CHECK(readBuffer[i] == i);
    }
    trueSize = 0;
    REQUIRE(ringBuffer.deleteData(5, false, &trueSize) == returnvalue::FAILED);
    REQUIRE(trueSize == 0);
    REQUIRE(ringBuffer.deleteData(5, true, &trueSize) == returnvalue::OK);
    REQUIRE(trueSize == 3);
  }

  SECTION("Overflow") {
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    // We don't allow writing of Data that is larger than the ring buffer in total
    REQUIRE(ringBuffer.getMaxSize() == 9);
    REQUIRE(ringBuffer.writeData(testData, 13) == returnvalue::FAILED);
    REQUIRE(ringBuffer.getAvailableReadData() == 0);
    ringBuffer.clear();
    uint8_t *ptr = nullptr;
    // With excess Bytes 13 Bytes can be written to this Buffer
    REQUIRE(ringBuffer.getFreeElement(&ptr, 13) == returnvalue::OK);
    REQUIRE(ptr != nullptr);
    memcpy(ptr, testData, 13);
    ringBuffer.confirmBytesWritten(13);
    REQUIRE(ringBuffer.getAvailableReadData() == 3);
    REQUIRE(ringBuffer.readData(readBuffer, 3, true) == returnvalue::OK);
    for (auto i = 0; i < 3; i++) {
      REQUIRE(readBuffer[i] == testData[i + 10]);
    }
  }
}

TEST_CASE("Ring Buffer Test3", "[RingBufferTest3]") {
  uint8_t testData[13] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  uint8_t readBuffer[10] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13};
  uint8_t *newBuffer = new uint8_t[25];
  SimpleRingBuffer ringBuffer(newBuffer, 10, true, 15);

  SECTION("Simple Test") {
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 9) == returnvalue::OK);
    REQUIRE(ringBuffer.readData(readBuffer, 5, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 5; i++) {
      CHECK(readBuffer[i] == i);
    }
    REQUIRE(ringBuffer.availableWriteSpace() == 5);
    ringBuffer.clear();
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 4) == returnvalue::OK);
    REQUIRE(ringBuffer.readData(readBuffer, 4, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 4; i++) {
      CHECK(readBuffer[i] == i);
    }
    REQUIRE(ringBuffer.writeData(testData, 9) == returnvalue::OK);
    REQUIRE(ringBuffer.readData(readBuffer, 9, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 9; i++) {
      CHECK(readBuffer[i] == i);
    }
  }

  SECTION("Get Free Element Test") {
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 8) == returnvalue::OK);
    REQUIRE(ringBuffer.availableWriteSpace() == 1);
    REQUIRE(ringBuffer.readData(readBuffer, 8, true) == returnvalue::OK);
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    uint8_t *testPtr = nullptr;
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 10) == returnvalue::OK);
    REQUIRE(ringBuffer.getExcessBytes() == 8);

    REQUIRE(ringBuffer.writeTillWrap() == 2);
    // too many excess bytes.
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 8) == returnvalue::FAILED);
    // Less Execss bytes overwrites before
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 3) == returnvalue::OK);
    REQUIRE(ringBuffer.getExcessBytes() == 1);
    std::memcpy(testPtr, testData, 3);
    ringBuffer.confirmBytesWritten(3);
    REQUIRE(ringBuffer.getAvailableReadData() == 3);
    ringBuffer.readData(readBuffer, 3, true);
    for (uint8_t i = 0; i < 3; i++) {
      CHECK(readBuffer[i] == i);
    }
  }

  SECTION("Read Remaining Test") {
    REQUIRE(ringBuffer.writeData(testData, 3) == returnvalue::OK);
    REQUIRE(ringBuffer.getAvailableReadData() == 3);
    REQUIRE(ringBuffer.readData(readBuffer, 5, false, false, nullptr) == returnvalue::FAILED);
    size_t trueSize = 0;
    REQUIRE(ringBuffer.readData(readBuffer, 5, false, true, &trueSize) == returnvalue::OK);
    REQUIRE(trueSize == 3);
    for (uint8_t i = 0; i < 3; i++) {
      CHECK(readBuffer[i] == i);
    }
    trueSize = 0;
    REQUIRE(ringBuffer.deleteData(5, false, &trueSize) == returnvalue::FAILED);
    REQUIRE(trueSize == 0);
    REQUIRE(ringBuffer.deleteData(5, true, &trueSize) == returnvalue::OK);
    REQUIRE(trueSize == 3);
  }

  SECTION("Overflow") {
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    // Writing more than the buffer is large.
    // This write will be rejected and is seen as a configuration mistake
    REQUIRE(ringBuffer.writeData(testData, 13) == returnvalue::FAILED);
    REQUIRE(ringBuffer.getAvailableReadData() == 0);
    ringBuffer.clear();
    // Using FreeElement allows the usage of excessBytes but
    // should be used with caution
    uint8_t *ptr = nullptr;
    REQUIRE(ringBuffer.getFreeElement(&ptr, 13) == returnvalue::OK);
    REQUIRE(ptr != nullptr);
    memcpy(ptr, testData, 13);
    ringBuffer.confirmBytesWritten(13);
    REQUIRE(ringBuffer.getAvailableReadData() == 3);
    REQUIRE(ringBuffer.readData(readBuffer, 3, true) == returnvalue::OK);
    for (auto i = 0; i < 3; i++) {
      REQUIRE(readBuffer[i] == testData[i + 10]);
    }
  }
}

TEST_CASE("Ring Buffer Test4", "[RingBufferTest4]") {
  uint8_t testData[13] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  uint8_t readBuffer[10] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13};
  SimpleRingBuffer ringBuffer(10, false, 15);

  SECTION("Simple Test") {
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 9) == returnvalue::OK);
    REQUIRE(ringBuffer.writeData(testData, 3) == returnvalue::FAILED);
    REQUIRE(ringBuffer.readData(readBuffer, 5, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 5; i++) {
      CHECK(readBuffer[i] == i);
    }
    REQUIRE(ringBuffer.availableWriteSpace() == 5);
    ringBuffer.clear();
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 4) == returnvalue::OK);
    REQUIRE(ringBuffer.readData(readBuffer, 4, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 4; i++) {
      CHECK(readBuffer[i] == i);
    }
    REQUIRE(ringBuffer.writeData(testData, 9) == returnvalue::OK);
    REQUIRE(ringBuffer.readData(readBuffer, 9, true) == returnvalue::OK);
    for (uint8_t i = 0; i < 9; i++) {
      CHECK(readBuffer[i] == i);
    }
  }

  SECTION("Get Free Element Test") {
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    REQUIRE(ringBuffer.writeData(testData, 8) == returnvalue::OK);
    REQUIRE(ringBuffer.availableWriteSpace() == 1);
    REQUIRE(ringBuffer.readData(readBuffer, 8, true) == returnvalue::OK);
    REQUIRE(ringBuffer.availableWriteSpace() == 9);
    uint8_t *testPtr = nullptr;
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 10) == returnvalue::FAILED);

    REQUIRE(ringBuffer.writeTillWrap() == 2);
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 8) == returnvalue::OK);
    REQUIRE(ringBuffer.getFreeElement(&testPtr, 5) == returnvalue::OK);
    REQUIRE(ringBuffer.getExcessBytes() == 3);
    std::memcpy(testPtr, testData, 5);
    ringBuffer.confirmBytesWritten(5);
    REQUIRE(ringBuffer.getAvailableReadData() == 5);
    ringBuffer.readData(readBuffer, 5, true);
    for (uint8_t i = 0; i < 5; i++) {
      CHECK(readBuffer[i] == i);
    }
  }

  SECTION("Read Remaining Test") {
    REQUIRE(ringBuffer.writeData(testData, 3) == returnvalue::OK);
    REQUIRE(ringBuffer.getAvailableReadData() == 3);
    REQUIRE(ringBuffer.readData(readBuffer, 5, false, false, nullptr) == returnvalue::FAILED);
    size_t trueSize = 0;
    REQUIRE(ringBuffer.readData(readBuffer, 5, false, true, &trueSize) == returnvalue::OK);
    REQUIRE(trueSize == 3);
    for (uint8_t i = 0; i < 3; i++) {
      CHECK(readBuffer[i] == i);
    }
    trueSize = 0;
    REQUIRE(ringBuffer.deleteData(5, false, &trueSize) == returnvalue::FAILED);
    REQUIRE(trueSize == 0);
    REQUIRE(ringBuffer.deleteData(5, true, &trueSize) == returnvalue::OK);
    REQUIRE(trueSize == 3);
  }
}
