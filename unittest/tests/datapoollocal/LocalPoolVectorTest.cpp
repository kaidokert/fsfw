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

	SECTION("BasicTest") {
		// very basic test.
		lp_vec_t<uint16_t, 3> testVector = lp_vec_t<uint16_t, 3>(
				objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint16Vec3Id);
		CHECK(testVector.read() == retval::CATCH_OK);
		testVector.value[0] = 5;
		testVector.value[1] = 232;
		testVector.value[2] = 32023;

		REQUIRE(testVector.commit(true) == retval::CATCH_OK);
		CHECK(testVector.isValid());

		testVector.value[0] = 0;
		testVector.value[1] = 0;
		testVector.value[2] = 0;

		CHECK(testVector.read() == retval::CATCH_OK);
		CHECK(testVector.value[0] == 5);
		CHECK(testVector.value[1] == 232);
		CHECK(testVector.value[2] == 32023);
	}
}


