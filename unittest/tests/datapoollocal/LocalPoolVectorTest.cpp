#include "LocalPoolOwnerBase.h"

#include <catch2/catch_test_macros.hpp>
#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <unittest/core/CatchDefinitions.h>

TEST_CASE("LocalPoolVector" , "[LocPoolVecTest]") {
	LocalPoolOwnerBase* poolOwner = objectManager->
			get<LocalPoolOwnerBase>(objects::TEST_LOCAL_POOL_OWNER_BASE);
	REQUIRE(poolOwner != nullptr);
	REQUIRE(poolOwner->initializeHkManager() == retval::CATCH_OK);
	REQUIRE(poolOwner->initializeHkManagerAfterTaskCreation()
			== retval::CATCH_OK);
}


