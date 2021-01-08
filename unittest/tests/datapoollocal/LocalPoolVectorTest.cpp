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
		REQUIRE(testVector.read() == retval::CATCH_OK);
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

		CHECK(testVector[0] == 5);

		// This is invalid access, so the last value will be set instead.
		// (we can't throw exceptions)
		testVector[4] = 12;
		CHECK(testVector[3] == 12);
		CHECK(testVector.commit() == retval::CATCH_OK);

		// Use read-only reference.
		const lp_vec_t<uint16_t, 3>& roTestVec = testVector;
		uint16_t valueOne = roTestVec[0];
		CHECK(valueOne == 5);

		uint16_t lastVal = roTestVec[25];
		CHECK(lastVal == 12);
	}

	SECTION("ErrorHandling") {
		// not try to use a local pool variable which does not exist
		lp_vec_t<uint16_t, 3> invalidVector = lp_vec_t<uint16_t, 3>(
				objects::TEST_LOCAL_POOL_OWNER_BASE, 0xffffffff);
		REQUIRE(invalidVector.read() ==
				static_cast<int>(HasLocalDataPoolIF::POOL_ENTRY_NOT_FOUND));
		REQUIRE(invalidVector.commit() ==
				static_cast<int>(HasLocalDataPoolIF::POOL_ENTRY_NOT_FOUND));

		// now try to access with wrong type
		lp_vec_t<uint32_t, 3> invalidVector2 = lp_vec_t<uint32_t, 3>(
				objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint16Vec3Id);
		REQUIRE(invalidVector2.read() ==
				static_cast<int>(HasLocalDataPoolIF::POOL_ENTRY_TYPE_CONFLICT));
	}
}


