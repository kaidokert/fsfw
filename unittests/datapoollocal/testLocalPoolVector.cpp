#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/objectmanager/ObjectManager.h>

#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"
#include "mocks/LocalPoolOwnerBase.h"
#include "tests/TestsConfig.h"

using namespace returnvalue;

TEST_CASE("LocalPoolVector", "[LocPoolVecTest]") {
  auto queue = MessageQueueMock(1);
  LocalPoolOwnerBase poolOwner(queue, objects::TEST_LOCAL_POOL_OWNER_BASE);
  REQUIRE(poolOwner.initializeHkManager() == OK);
  REQUIRE(poolOwner.initializeHkManagerAfterTaskCreation() == OK);

  SECTION("BasicTest") {
    // very basic test.
    lp_vec_t<uint16_t, 3> testVector =
        lp_vec_t<uint16_t, 3>(objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint16Vec3Id);
    REQUIRE(testVector.read() == returnvalue::OK);
    testVector.value[0] = 5;
    testVector.value[1] = 232;
    testVector.value[2] = 32023;

    REQUIRE(testVector.commit(true) == returnvalue::OK);
    CHECK(testVector.isValid());

    testVector.value[0] = 0;
    testVector.value[1] = 0;
    testVector.value[2] = 0;

    CHECK(testVector.read() == returnvalue::OK);
    CHECK(testVector.value[0] == 5);
    CHECK(testVector.value[1] == 232);
    CHECK(testVector.value[2] == 32023);

    CHECK(testVector[0] == 5);

    /* This is invalid access, so the last value will be set instead.
    (we can't throw exceptions) */
    testVector[4] = 12;
    CHECK(testVector[2] == 12);
    CHECK(testVector.commit() == returnvalue::OK);

    /* Use read-only reference. */
    const lp_vec_t<uint16_t, 3>& roTestVec = testVector;
    uint16_t valueOne = roTestVec[0];
    CHECK(valueOne == 5);

    uint16_t lastVal = roTestVec[25];
    CHECK(lastVal == 12);

    size_t maxSize = testVector.getSerializedSize();
    CHECK(maxSize == 6);

    uint16_t serializedVector[3];
    uint8_t* vecPtr = reinterpret_cast<uint8_t*>(serializedVector);
    size_t serSize = 0;
    REQUIRE(testVector.serialize(&vecPtr, &serSize, maxSize, SerializeIF::Endianness::MACHINE) ==
            returnvalue::OK);

    CHECK(serSize == 6);
    CHECK(serializedVector[0] == 5);
    CHECK(serializedVector[1] == 232);
    CHECK(serializedVector[2] == 12);

    maxSize = 1;
    REQUIRE(testVector.serialize(&vecPtr, &serSize, maxSize, SerializeIF::Endianness::MACHINE) ==
            static_cast<int>(SerializeIF::BUFFER_TOO_SHORT));

    serializedVector[0] = 16;
    serializedVector[1] = 7832;
    serializedVector[2] = 39232;

    const uint8_t* constVecPtr = reinterpret_cast<const uint8_t*>(serializedVector);
    REQUIRE(testVector.deSerialize(&constVecPtr, &serSize, SerializeIF::Endianness::MACHINE) ==
            returnvalue::OK);
    CHECK(testVector[0] == 16);
    CHECK(testVector[1] == 7832);
    CHECK(testVector[2] == 39232);

    serSize = 1;
    REQUIRE(testVector.deSerialize(&constVecPtr, &serSize, SerializeIF::Endianness::MACHINE) ==
            static_cast<int>(SerializeIF::STREAM_TOO_SHORT));
  }

  SECTION("ErrorHandling") {
    /* Now try to use a local pool variable which does not exist */
    lp_vec_t<uint16_t, 3> invalidVector =
        lp_vec_t<uint16_t, 3>(objects::TEST_LOCAL_POOL_OWNER_BASE, 0xffffffff);
    REQUIRE(invalidVector.read() == static_cast<int>(localpool::POOL_ENTRY_NOT_FOUND));
    REQUIRE(invalidVector.commit() == static_cast<int>(localpool::POOL_ENTRY_NOT_FOUND));

    /* Now try to access with wrong type */
    lp_vec_t<uint32_t, 3> invalidVector2 =
        lp_vec_t<uint32_t, 3>(objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint16Vec3Id);
    REQUIRE(invalidVector2.read() == static_cast<int>(localpool::POOL_ENTRY_TYPE_CONFLICT));
    REQUIRE(invalidVector2.commit() == static_cast<int>(localpool::POOL_ENTRY_TYPE_CONFLICT));

    lp_vec_t<uint16_t, 3> writeOnlyVec = lp_vec_t<uint16_t, 3>(
        objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint16Vec3Id, nullptr, pool_rwm_t::VAR_WRITE);
    REQUIRE(writeOnlyVec.read() == static_cast<int>(PoolVariableIF::INVALID_READ_WRITE_MODE));

    lp_vec_t<uint16_t, 3> readOnlyVec = lp_vec_t<uint16_t, 3>(
        objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint16Vec3Id, nullptr, pool_rwm_t::VAR_READ);
    REQUIRE(readOnlyVec.commit() == static_cast<int>(PoolVariableIF::INVALID_READ_WRITE_MODE));
  }
}
