#include "InternalUnitTester.h"
#include "UnittDefinitions.h"

#include "osal/IntTestMq.h"
#include "osal/IntTestSemaphore.h"
#include "osal/IntTestMutex.h"
#include "serialize/IntTestSerialization.h"
#include "globalfunctions/TestArrayPrinter.h"

#include <cstdlib>

struct TestConfig {
    bool testArrayPrinter;
};
InternalUnitTester::InternalUnitTester() {}

InternalUnitTester::~InternalUnitTester() {}

ReturnValue_t InternalUnitTester::performTests(struct TestConfig testConfig) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
	sif::info << "Running internal unit tests.." << std::endl;
#else
	sif::printInfo("Running internal unit tests..\n");
#endif

	testserialize::test_serialization();
	testmq::testMq();
	testsemaph::testBinSemaph();
	testsemaph::testCountingSemaph();
	testmutex::testMutex();
	if(testConfig.testArrayPrinter) {
	    arrayprinter::testArrayPrinter();
	}

#if FSFW_CPP_OSTREAM_ENABLED == 1
	sif::info << "Internal unit tests finished." << std::endl;
#else
	sif::printInfo("Running internal unit tests..\n");
#endif
	return RETURN_OK;
}



