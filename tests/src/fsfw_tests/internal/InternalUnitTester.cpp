#include "fsfw_tests/internal/InternalUnitTester.h"
#include "fsfw_tests/internal/UnittDefinitions.h"

#include "fsfw_tests/internal/osal/IntTestMq.h"
#include "fsfw_tests/internal/osal/IntTestSemaphore.h"
#include "fsfw_tests/internal/osal/IntTestMutex.h"
#include "fsfw_tests/internal/serialize/IntTestSerialization.h"
#include "fsfw_tests/internal/globalfunctions/TestArrayPrinter.h"

#include <cstdlib>

InternalUnitTester::InternalUnitTester() {}

InternalUnitTester::~InternalUnitTester() {}

ReturnValue_t InternalUnitTester::performTests(
        const struct InternalUnitTester::TestConfig& testConfig) {
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
	sif::printInfo("Internal unit tests finished.\n");
#endif
	return RETURN_OK;
}



