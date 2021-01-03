#include "LocalPoolOwnerBase.h"

#include <catch2/catch_test_macros.hpp>
#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <unittest/core/CatchDefinitions.h>


TEST_CASE("LocalPoolVariable" , "[LocPoolVarTest]") {
	LocalPoolOwnerBase poolOwner(objects::TEST_LOCAL_POOL_OWNER_BASE);
	REQUIRE(poolOwner.initializeHkManager() == retval::CATCH_OK);
	REQUIRE(poolOwner.initializeHkManagerAfterTaskCreation()
			== retval::CATCH_OK);

	// very basic test.
	lp_var_t<uint8_t> testVariable = lp_var_t<uint8_t>(
			objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint8VarId);
	REQUIRE(testVariable.read() == retval::CATCH_OK);
	CHECK(testVariable.value == 0);
	testVariable.value = 5;
	REQUIRE(testVariable.commit() == retval::CATCH_OK);
	REQUIRE(testVariable.read() == retval::CATCH_OK);
	REQUIRE(testVariable.value == 5);

	// not try to use a local pool variable which does not exist
	lp_var_t<uint8_t> invalidVariable = lp_var_t<uint8_t>(
			objects::TEST_LOCAL_POOL_OWNER_BASE, 0xffffffff);
	REQUIRE(invalidVariable.read() ==
			static_cast<int>(HasLocalDataPoolIF::POOL_ENTRY_NOT_FOUND));

	// now try to access with wrong type
	lp_var_t<int8_t> invalidVariable2 = lp_var_t<int8_t>(
			objects::TEST_LOCAL_POOL_OWNER_BASE, lpool::uint8VarId);
	REQUIRE(invalidVariable2.read() ==
			static_cast<int>(HasLocalDataPoolIF::POOL_ENTRY_TYPE_CONFLICT));
}


