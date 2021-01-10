#include "LocalPoolOwnerBase.h"

#include <catch2/catch_test_macros.hpp>
#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <unittest/core/CatchDefinitions.h>


TEST_CASE("LocalPoolManagerTest" , "[LocManTest]") {
	LocalPoolOwnerBase* poolOwner = objectManager->
			get<LocalPoolOwnerBase>(objects::TEST_LOCAL_POOL_OWNER_BASE);
	REQUIRE(poolOwner != nullptr);
	REQUIRE(poolOwner->initializeHkManager() == retval::CATCH_OK);
	REQUIRE(poolOwner->initializeHkManagerAfterTaskCreation()
			== retval::CATCH_OK);
	REQUIRE(poolOwner->dataset.assignPointers() == retval::CATCH_OK);
	MessageQueueMockBase* mqMock = poolOwner->getMockQueueHandle();
	REQUIRE(mqMock != nullptr);

	poolOwner->subscribeWrapperSetUpdate();
	SECTION("BasicTest") {
		poolOwner->dataset.setChanged(true);
		REQUIRE(poolOwner->hkManager.performHkOperation() == retval::CATCH_OK);
		REQUIRE(mqMock->wasMessageSent() == true);

	}
}

