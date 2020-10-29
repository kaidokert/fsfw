#include "InternalUnitTester.h"
#include "UnittDefinitions.h"

#include "osal/IntTestMq.h"
#include "osal/IntTestSemaphore.h"
#include "osal/IntTestMutex.h"
#include "serialize/IntTestSerialization.h"

#include <cstdlib>

InternalUnitTester::InternalUnitTester() {}

InternalUnitTester::~InternalUnitTester() {}

ReturnValue_t InternalUnitTester::performTests() {
	sif::info << "Running internal unit tests.." << std::endl;
	testserialize::test_serialization();
	testmq::testMq();
	testsemaph::testBinSemaph();
	testsemaph::testCountingSemaph();
	testmutex::testMutex();
	sif::info << "Internal unit tests finished." << std::endl;
	return RETURN_OK;
}



