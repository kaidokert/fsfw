#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/storagemanager/LocalPool.h>

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstring>

#include "CatchDefinitions.h"

TEST_CASE("Local Pool Simple Tests [1 Pool]", "[TestPool]") {
  LocalPool::LocalPoolConfig config = {{1, 10}};
  LocalPool simplePool(0, config);
  std::array<uint8_t, 20> testDataArray{};
  std::array<uint8_t, 20> receptionArray{};
  store_address_t testStoreId;
  ReturnValue_t result;
  uint8_t* pointer = nullptr;
  const uint8_t* constPointer = nullptr;

  for (size_t i = 0; i < testDataArray.size(); i++) {
    testDataArray[i] = i;
  }
  size_t size = 10;

  SECTION("Basic tests") {
    REQUIRE(not simplePool.hasDataAtId(testStoreId));
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(simplePool.hasDataAtId(testStoreId));
    result = simplePool.getData(testStoreId, &constPointer, &size);
    REQUIRE(result == returnvalue::OK);
    memcpy(receptionArray.data(), constPointer, size);
    for (size_t i = 0; i < size; i++) {
      CHECK(receptionArray[i] == i);
    }
    memset(receptionArray.data(), 0, size);
    result = simplePool.modifyData(testStoreId, &pointer, &size);
    memcpy(receptionArray.data(), pointer, size);
    REQUIRE(result == returnvalue::OK);
    for (size_t i = 0; i < size; i++) {
      CHECK(receptionArray[i] == i);
    }
    result = simplePool.deleteData(testStoreId);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(not simplePool.hasDataAtId(testStoreId));
    result = simplePool.addData(&testStoreId, testDataArray.data(), 15);
    CHECK(result == (int)StorageManagerIF::DATA_TOO_LARGE);
  }

  SECTION("Reservation Tests ") {
    pointer = nullptr;
    result = simplePool.getFreeElement(&testStoreId, size, &pointer);
    REQUIRE(result == returnvalue::OK);
    memcpy(pointer, testDataArray.data(), size);
    constPointer = nullptr;
    result = simplePool.getData(testStoreId, &constPointer, &size);

    REQUIRE(result == returnvalue::OK);
    memcpy(receptionArray.data(), constPointer, size);
    for (size_t i = 0; i < size; i++) {
      CHECK(receptionArray[i] == i);
    }
  }

  SECTION("Add, delete, add, add when full") {
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    result = simplePool.getData(testStoreId, &constPointer, &size);
    REQUIRE(result == returnvalue::OK);
    memcpy(receptionArray.data(), constPointer, size);
    for (size_t i = 0; i < size; i++) {
      CHECK(receptionArray[i] == i);
    }

    result = simplePool.deleteData(testStoreId);
    REQUIRE(result == returnvalue::OK);

    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    result = simplePool.getData(testStoreId, &constPointer, &size);
    REQUIRE(result == returnvalue::OK);
    memcpy(receptionArray.data(), constPointer, size);
    for (size_t i = 0; i < size; i++) {
      CHECK(receptionArray[i] == i);
    }

    store_address_t newAddress;
    result = simplePool.addData(&newAddress, testDataArray.data(), size);
    REQUIRE(result == (int)StorageManagerIF::DATA_STORAGE_FULL);

    // Packet Index to high intentionally
    newAddress.packetIndex = 2;
    pointer = testDataArray.data();
    result = simplePool.modifyData(newAddress, &pointer, &size);
    REQUIRE(result == (int)StorageManagerIF::ILLEGAL_STORAGE_ID);

    result = simplePool.deleteData(newAddress);
    REQUIRE(result == (int)StorageManagerIF::ILLEGAL_STORAGE_ID);

    newAddress.packetIndex = 0;
    newAddress.poolIndex = 2;
    result = simplePool.deleteData(newAddress);
    REQUIRE(result == (int)StorageManagerIF::ILLEGAL_STORAGE_ID);
  }

  SECTION("Initialize and clear store, delete with pointer") {
    result = simplePool.initialize();
    REQUIRE(result == returnvalue::OK);
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    simplePool.clearStore();
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    result = simplePool.modifyData(testStoreId, &pointer, &size);
    REQUIRE(result == returnvalue::OK);
    store_address_t newId;
    result = simplePool.deleteData(pointer, size, &testStoreId);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(testStoreId.raw != (uint32_t)StorageManagerIF::INVALID_ADDRESS);
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
  }
}

int runIdx = 0;

TEST_CASE("Local Pool Extended Tests [3 Pools]", "[TestPool2]") {
  LocalPool::LocalPoolConfig* config;
  if (runIdx == 0) {
    config = new LocalPool::LocalPoolConfig{{10, 5}, {5, 10}, {2, 20}};
  } else {
    // shufle the order, they should be sort implictely so that the
    // order is ascending for the page sizes.
    config = new LocalPool::LocalPoolConfig{{5, 10}, {2, 20}, {10, 5}};
    size_t lastSize = 0;
    for (const auto& pair : *config) {
      CHECK(pair.second > lastSize);
      lastSize = pair.second;
    }
  }
  runIdx++;

  LocalPool simplePool(0, *config);
  std::array<uint8_t, 20> testDataArray{};
  std::array<uint8_t, 20> receptionArray{};
  store_address_t testStoreId;
  ReturnValue_t result = returnvalue::FAILED;
  for (size_t i = 0; i < testDataArray.size(); i++) {
    testDataArray[i] = i;
  }
  size_t size = 0;

  SECTION("Basic tests") {
    size = 8;
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    // Should be on second page of the pool now for 8 bytes
    CHECK(testStoreId.poolIndex == 1);
    CHECK(testStoreId.packetIndex == 0);

    size = 15;
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    // Should be on third page of the pool now for 15 bytes
    CHECK(testStoreId.poolIndex == 2);
    CHECK(testStoreId.packetIndex == 0);

    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    // Should be on third page of the pool now for 15 bytes
    CHECK(testStoreId.poolIndex == 2);
    CHECK(testStoreId.packetIndex == 1);

    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    // Should be on third page of the pool now for 15 bytes
    REQUIRE(result == (int)LocalPool::DATA_STORAGE_FULL);

    size = 8;
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    // Should still work
    CHECK(testStoreId.poolIndex == 1);
    CHECK(testStoreId.packetIndex == 1);

    // fill the rest of the pool
    for (uint8_t idx = 2; idx < 5; idx++) {
      result = simplePool.addData(&testStoreId, testDataArray.data(), size);
      REQUIRE(result == returnvalue::OK);
      CHECK(testStoreId.poolIndex == 1);
      CHECK(testStoreId.packetIndex == idx);
    }
  }

  SECTION("Fill Count and Clearing") {
    // SECTION("Basic tests");
    uint8_t bytesWritten = 0;
    simplePool.getFillCount(receptionArray.data(), &bytesWritten);
    // fill count should be all zeros now.
    CHECK(bytesWritten == 4);
    CHECK(receptionArray[0] == 0);
    CHECK(receptionArray[1] == 0);
    CHECK(receptionArray[2] == 0);
    CHECK(receptionArray[3] == 0);

    // now fill the store completely.
    size = 5;
    for (uint8_t idx = 0; idx < 10; idx++) {
      result = simplePool.addData(&testStoreId, testDataArray.data(), size);
      REQUIRE(result == returnvalue::OK);
      CHECK(testStoreId.poolIndex == 0);
      CHECK(testStoreId.packetIndex == idx);
    }
    size = 10;
    for (uint8_t idx = 0; idx < 5; idx++) {
      result = simplePool.addData(&testStoreId, testDataArray.data(), size);
      REQUIRE(result == returnvalue::OK);
      CHECK(testStoreId.poolIndex == 1);
      CHECK(testStoreId.packetIndex == idx);
    }
    size = 20;
    for (uint8_t idx = 0; idx < 2; idx++) {
      result = simplePool.addData(&testStoreId, testDataArray.data(), size);
      REQUIRE(result == returnvalue::OK);
      CHECK(testStoreId.poolIndex == 2);
      CHECK(testStoreId.packetIndex == idx);
    }
    bytesWritten = 0;
    simplePool.getFillCount(receptionArray.data(), &bytesWritten);
    // fill count should be all 100 now.
    CHECK(bytesWritten == 4);
    CHECK(receptionArray[0] == 100);
    CHECK(receptionArray[1] == 100);
    CHECK(receptionArray[2] == 100);
    CHECK(receptionArray[3] == 100);

    // now clear the store
    simplePool.clearStore();
    bytesWritten = 0;
    simplePool.getFillCount(receptionArray.data(), &bytesWritten);
    CHECK(bytesWritten == 4);
    CHECK(receptionArray[0] == 0);
    CHECK(receptionArray[1] == 0);
    CHECK(receptionArray[2] == 0);
    CHECK(receptionArray[3] == 0);

    // now fill one page
    size = 5;
    for (uint8_t idx = 0; idx < 10; idx++) {
      result = simplePool.addData(&testStoreId, testDataArray.data(), size);
      REQUIRE(result == returnvalue::OK);
      CHECK(testStoreId.poolIndex == 0);
      CHECK(testStoreId.packetIndex == idx);
    }
    bytesWritten = 0;
    simplePool.getFillCount(receptionArray.data(), &bytesWritten);
    // First page full, median fill count is 33 %
    CHECK(bytesWritten == 4);
    CHECK(receptionArray[0] == 100);
    CHECK(receptionArray[1] == 0);
    CHECK(receptionArray[2] == 0);
    CHECK(receptionArray[3] == 33);

    // now fill second page
    size = 10;
    for (uint8_t idx = 0; idx < 5; idx++) {
      result = simplePool.addData(&testStoreId, testDataArray.data(), size);
      REQUIRE(result == returnvalue::OK);
      CHECK(testStoreId.poolIndex == 1);
      CHECK(testStoreId.packetIndex == idx);
    }
    bytesWritten = 0;
    simplePool.getFillCount(receptionArray.data(), &bytesWritten);
    // First and second page full, median fill count is 66 %
    CHECK(bytesWritten == 4);
    CHECK(receptionArray[0] == 100);
    CHECK(receptionArray[1] == 100);
    CHECK(receptionArray[2] == 0);
    CHECK(receptionArray[3] == 66);

    // now clear first page
    simplePool.clearSubPool(0);
    bytesWritten = 0;
    simplePool.getFillCount(receptionArray.data(), &bytesWritten);
    // Second page full, median fill count is 33 %
    CHECK(bytesWritten == 4);
    CHECK(receptionArray[0] == 0);
    CHECK(receptionArray[1] == 100);
    CHECK(receptionArray[2] == 0);
    CHECK(receptionArray[3] == 33);
  }

  delete (config);
}
