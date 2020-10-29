#include "IntTestSemaphore.h"
#include <fsfw/tasks/SemaphoreFactory.h>
#include <unittest/internal/UnittDefinitions.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/timemanager/Stopwatch.h>


void testsemaph::testBinSemaph() {
	std::string id = "[BinSemaphore]";
	SemaphoreIF* binSemaph =
			SemaphoreFactory::instance()->createBinarySemaphore();
	if(binSemaph == nullptr) {
		return;
	}
	testBinSemaphoreImplementation(binSemaph, id);
	SemaphoreFactory::instance()->deleteSemaphore(binSemaph);
#if defined(freeRTOS)
	SemaphoreIF* binSemaphUsingTask =
			SemaphoreFactory::instance()->createBinarySemaphore(1);
	testBinSemaphoreImplementation(binSemaphUsingTask, id);
	SemaphoreFactory::instance()->deleteSemaphore(binSemaphUsingTask);
#endif
}


void testsemaph::testCountingSemaph() {
	std::string id = "[CountingSemaph]";
	{
		// First test: create a binary semaphore by using a counting semaphore.
		SemaphoreIF* countingSemaph = SemaphoreFactory::instance()->
				createCountingSemaphore(1,1);
		if(countingSemaph == nullptr) {
			return;
		}
		testBinSemaphoreImplementation(countingSemaph, id);
		SemaphoreFactory::instance()->deleteSemaphore(countingSemaph);
#if defined(freeRTOS)
		countingSemaph = SemaphoreFactory::instance()->
				createCountingSemaphore(1, 1, 1);
		testBinSemaphoreImplementation(countingSemaph, id);
		SemaphoreFactory::instance()->deleteSemaphore(countingSemaph);
#endif
	}

	{
		// Second test: counting semaphore with count 3 and init count of 3.
		SemaphoreIF* countingSemaph = SemaphoreFactory::instance()->
				createCountingSemaphore(3,3);
		testCountingSemaphImplementation(countingSemaph, id);
		SemaphoreFactory::instance()->deleteSemaphore(countingSemaph);
#if defined(freeRTOS)
		countingSemaph = SemaphoreFactory::instance()->
				createCountingSemaphore(3, 0, 1);
		uint8_t semaphCount = countingSemaph->getSemaphoreCounter();
		if(semaphCount != 0) {
			unitt::put_error(id);
		}
		// release 3 times in a row
		for(int i = 0; i < 3; i++) {
			auto result = countingSemaph->release();
			if(result != HasReturnvaluesIF::RETURN_OK) {
				unitt::put_error(id);
			}
		}
		testCountingSemaphImplementation(countingSemaph, id);
		SemaphoreFactory::instance()->deleteSemaphore(countingSemaph);
#endif
	}
}


void testsemaph::testBinSemaphoreImplementation(SemaphoreIF* binSemaph,
		std::string id) {
	uint8_t semaphCount = binSemaph->getSemaphoreCounter();
	if(semaphCount != 1) {
		unitt::put_error(id);
	}

	ReturnValue_t result = binSemaph->release();
	if(result != SemaphoreIF::SEMAPHORE_NOT_OWNED) {
		unitt::put_error(id);
	}
	result = binSemaph->acquire(SemaphoreIF::BLOCKING);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		unitt::put_error(id);
	}

	// There is not really a point in testing time related, the task
	// might get interrupted..
	{
		//Stopwatch stopwatch(false);
		result = binSemaph->acquire(SemaphoreIF::TimeoutType::WAITING, 10);
		//dur_millis_t time = stopwatch.stop();
//		if(abs(time - 10) > 2) {
//		    sif::error << "UnitTester: Semaphore timeout measured incorrect."
//		            << std::endl;
//			unitt::put_error(id);
//		}
	}

	if(result != SemaphoreIF::SEMAPHORE_TIMEOUT) {
		unitt::put_error(id);
	}

	semaphCount = binSemaph->getSemaphoreCounter();
	if(semaphCount != 0) {
		unitt::put_error(id);
	}

	result = binSemaph->release();
	if(result != HasReturnvaluesIF::RETURN_OK) {
		unitt::put_error(id);
	}
}

void testsemaph::testCountingSemaphImplementation(SemaphoreIF* countingSemaph,
		std::string id) {
	// check count getter function
	uint8_t semaphCount = countingSemaph->getSemaphoreCounter();
	if(semaphCount != 3) {
		unitt::put_error(id);
	}
	ReturnValue_t result = countingSemaph->release();
	if(result != SemaphoreIF::SEMAPHORE_NOT_OWNED) {
		unitt::put_error(id);
	}
	// acquire 3 times in a row
	for(int i = 0; i < 3; i++) {
		result = countingSemaph->acquire(SemaphoreIF::BLOCKING);
		if(result != HasReturnvaluesIF::RETURN_OK) {
			unitt::put_error(id);
		}
	}

	{
		Stopwatch stopwatch(false);
		// attempt to take when count is 0, measure time
		result = countingSemaph->acquire(SemaphoreIF::TimeoutType::WAITING, 10);
		dur_millis_t time = stopwatch.stop();
		if(abs(time - 10) > 1) {
			unitt::put_error(id);
		}
	}

	if(result != SemaphoreIF::SEMAPHORE_TIMEOUT) {
		unitt::put_error(id);
	}

	// release 3 times in a row
	for(int i = 0; i < 3; i++) {
		result = countingSemaph->release();
		if(result != HasReturnvaluesIF::RETURN_OK) {
			unitt::put_error(id);
		}
	}
	// assert correct full count
	if(countingSemaph->getSemaphoreCounter() != 3) {
		unitt::put_error(id);
	}
}
