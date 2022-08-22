#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/objectmanager/ObjectManager.h>

#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"
#include "mocks/LocalPoolOwnerBase.h"
#include "tests/TestsConfig.h"

using namespace returnvalue;

TEST_CASE("LocalPoolVariable", "[LocPoolVarTest]") {
  auto queue = MessageQueueMock(1);
  LocalPoolOwnerBase poolOwner(queue, objects::TEST_LOCAL_POOL_OWNER_BASE);
  REQUIRE(poolOwner.initializeHkManager() == OK);
  REQUIRE(poolOwner.initializeHkManagerAfterTaskCreation() == OK);

  SECTION("Basic Tests") {
    /* very basic test. */
    lp_var_t<uint8_t> testVariable =
        lp_var_t<uint8_t>(objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint8VarId);
    REQUIRE(testVariable.read() == returnvalue::OK);
    CHECK(testVariable.value == 0);
    testVariable.value = 5;
    REQUIRE(testVariable.commit() == returnvalue::OK);
    REQUIRE(testVariable.read() == returnvalue::OK);
    REQUIRE(testVariable.value == 5);
    CHECK(not testVariable.isValid());
    testVariable.setValid(true);
    CHECK(testVariable.isValid());
    CHECK(testVariable.commit(true) == returnvalue::OK);

    testVariable.setReadWriteMode(pool_rwm_t::VAR_READ);
    CHECK(testVariable.getReadWriteMode() == pool_rwm_t::VAR_READ);
    testVariable.setReadWriteMode(pool_rwm_t::VAR_READ_WRITE);

    testVariable.setDataPoolId(22);
    CHECK(testVariable.getDataPoolId() == 22);
    testVariable.setDataPoolId(lpool::uint8VarId);

    testVariable.setChanged(true);
    CHECK(testVariable.hasChanged());
    testVariable.setChanged(false);

    gp_id_t globPoolId(objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint8VarId);
    lp_var_t<uint8_t> testVariable2 = lp_var_t<uint8_t>(globPoolId);
    REQUIRE(testVariable2.read() == returnvalue::OK);
    CHECK(testVariable2 == 5);
    CHECK(testVariable == testVariable2);
    testVariable = 10;
    CHECK(testVariable != 5);
    // CHECK(not testVariable != testVariable2);
    uint8_t variableRaw = 0;
    uint8_t* varPtr = &variableRaw;
    size_t maxSize = testVariable.getSerializedSize();
    CHECK(maxSize == 1);
    size_t serSize = 0;
    CHECK(testVariable.serialize(&varPtr, &serSize, maxSize, SerializeIF::Endianness::MACHINE) ==
          returnvalue::OK);
    CHECK(variableRaw == 10);
    const uint8_t* varConstPtr = &variableRaw;
    testVariable = 5;
    CHECK(testVariable.deSerialize(&varConstPtr, &serSize, SerializeIF::Endianness::MACHINE) ==
          returnvalue::OK);
    CHECK(testVariable == 10);
    CHECK(testVariable != testVariable2);
    CHECK(testVariable2 < testVariable);
    CHECK(testVariable2 < 10);
    CHECK(testVariable > 5);
    CHECK(testVariable > testVariable2);
    variableRaw = static_cast<uint8_t>(testVariable2);
    CHECK(variableRaw == 5);

    CHECK(testVariable == 10);
    testVariable = testVariable2;
    CHECK(testVariable == 5);
  }

  SECTION("ErrorHandling") {
    /* now try to use a local pool variable which does not exist */
    lp_var_t<uint8_t> invalidVariable =
        lp_var_t<uint8_t>(objects::TEST_LOCAL_POOL_OWNER_BASE, 0xffffffff);
    REQUIRE(invalidVariable.read() == static_cast<int>(localpool::POOL_ENTRY_NOT_FOUND));

    REQUIRE(invalidVariable.commit() == static_cast<int>(localpool::POOL_ENTRY_NOT_FOUND));
    /* now try to access with wrong type */
    lp_var_t<int8_t> invalidVariable2 =
        lp_var_t<int8_t>(objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint8VarId);
    REQUIRE(invalidVariable2.read() == static_cast<int>(localpool::POOL_ENTRY_TYPE_CONFLICT));

    lp_var_t<uint8_t> readOnlyVar = lp_var_t<uint8_t>(
        objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint8VarId, nullptr, pool_rwm_t::VAR_READ);
    REQUIRE(readOnlyVar.commit() == static_cast<int>(PoolVariableIF::INVALID_READ_WRITE_MODE));
    lp_var_t<uint8_t> writeOnlyVar = lp_var_t<uint8_t>(
        objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint8VarId, nullptr, pool_rwm_t::VAR_WRITE);
    REQUIRE(writeOnlyVar.read() == static_cast<int>(PoolVariableIF::INVALID_READ_WRITE_MODE));

    lp_var_t<uint32_t> uint32tVar =
        lp_var_t<uint32_t>(objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint32VarId);
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "LocalPoolVariable printout: " << uint32tVar << std::endl;
#endif

    /* for code coverage. If program does not crash -> OK */
    lp_var_t<uint8_t> invalidObjectVar = lp_var_t<uint8_t>(0xffffffff, lpool::uint8VarId);
    gp_id_t globPoolId(0xffffffff, lpool::uint8VarId);
    lp_var_t<uint8_t> invalidObjectVar2 = lp_var_t<uint8_t>(globPoolId);
    lp_var_t<uint8_t> invalidObjectVar3 = lp_var_t<uint8_t>(nullptr, lpool::uint8VarId);
  }
}
