#include "LocalPoolOwnerBase.h"

#include <catch2/catch_test_macros.hpp>
#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <unittest/core/CatchDefinitions.h>

TEST_CASE("LocalDataSet" , "[LocDataSetTest]") {
    LocalPoolOwnerBase* poolOwner = objectManager->
            get<LocalPoolOwnerBase>(objects::TEST_LOCAL_POOL_OWNER_BASE);
    REQUIRE(poolOwner != nullptr);
    REQUIRE(poolOwner->initializeHkManager() == retval::CATCH_OK);
    REQUIRE(poolOwner->initializeHkManagerAfterTaskCreation()
            == retval::CATCH_OK);
    const uint32_t setId = 0;
    SECTION("BasicTest") {
        StaticLocalDataSet<3> localSet = StaticLocalDataSet<3>(
                sid_t(objects::TEST_LOCAL_POOL_OWNER_BASE, setId));
    }
}



