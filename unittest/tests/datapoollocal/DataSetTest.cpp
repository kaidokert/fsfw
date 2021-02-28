#include "LocalPoolOwnerBase.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/datapool/PoolReadHelper.h>
#include <unittest/core/CatchDefinitions.h>

TEST_CASE("LocalDataSet" , "[LocDataSetTest]") {
    LocalPoolOwnerBase* poolOwner = objectManager->
            get<LocalPoolOwnerBase>(objects::TEST_LOCAL_POOL_OWNER_BASE);
    REQUIRE(poolOwner != nullptr);
    REQUIRE(poolOwner->initializeHkManager() == retval::CATCH_OK);
    REQUIRE(poolOwner->initializeHkManagerAfterTaskCreation()
            == retval::CATCH_OK);
    LocalPoolStaticTestDataSet localSet;

    SECTION("BasicTest") {
        {
            PoolReadHelper readHelper(&localSet);
            REQUIRE(readHelper.getReadResult() == retval::CATCH_OK);
            CHECK(not localSet.isValid());
            CHECK(localSet.localPoolVarUint8.value == 0);
            CHECK(not localSet.localPoolVarUint8.isValid());
            CHECK(localSet.localPoolVarFloat.value == Catch::Approx(0.0));
            CHECK(not localSet.localPoolVarUint8.isValid());
            CHECK(localSet.localPoolUint16Vec.value[0] == 0);
            CHECK(localSet.localPoolUint16Vec.value[1] == 0);
            CHECK(localSet.localPoolUint16Vec.value[2] == 0);
            CHECK(not localSet.localPoolVarUint8.isValid());

            localSet.localPoolVarUint8 = 232;
            localSet.localPoolVarFloat = -2324.322;
            localSet.localPoolUint16Vec.value[0] = 232;
            localSet.localPoolUint16Vec.value[1] = 23923;
            localSet.localPoolUint16Vec.value[2] = 1;
            localSet.setValidity(true, true);
        }

        {
            PoolReadHelper readHelper(&localSet);
            REQUIRE(readHelper.getReadResult() == retval::CATCH_OK);
            CHECK(localSet.isValid());
            CHECK(localSet.localPoolVarUint8.value == 232);
            CHECK(localSet.localPoolVarUint8.isValid());
            CHECK(localSet.localPoolVarFloat.value == Catch::Approx(-2324.322));
            CHECK(localSet.localPoolVarUint8.isValid());
            CHECK(localSet.localPoolUint16Vec.value[0] == 232);
            CHECK(localSet.localPoolUint16Vec.value[1] == 23923);
            CHECK(localSet.localPoolUint16Vec.value[2] == 1);
            CHECK(localSet.localPoolVarUint8.isValid());
        }

        /* Common fault test cases */
        LocalPoolObjectBase* variableHandle = poolOwner->getPoolObjectHandle(lpool::uint32VarId);
        CHECK(variableHandle != nullptr);
        CHECK(localSet.registerVariable(variableHandle) ==
                static_cast<int>(DataSetIF::DATA_SET_FULL));
        variableHandle = nullptr;
        REQUIRE(localSet.registerVariable(variableHandle) ==
                static_cast<int>(DataSetIF::POOL_VAR_NULL));
    }

    /* we need to reset the subscription list because the pool owner
    is a global object. */
    CHECK(poolOwner->reset() == retval::CATCH_OK);
}



