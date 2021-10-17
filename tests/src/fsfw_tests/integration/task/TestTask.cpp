#include "TestTask.h"

#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

bool TestTask::oneShotAction = true;
MutexIF* TestTask::testLock = nullptr;

TestTask::TestTask(object_id_t objectId, bool periodicPrintout, bool periodicEvent):
	SystemObject(objectId), testMode(testModes::A),
	periodicPrinout(periodicPrintout), periodicEvent(periodicEvent) {
	if(testLock == nullptr) {
		testLock = MutexFactory::instance()->createMutex();
	}
	IPCStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
}

TestTask::~TestTask() {
}

ReturnValue_t TestTask::performOperation(uint8_t operationCode) {
	ReturnValue_t result = RETURN_OK;
	testLock->lockMutex(MutexIF::TimeoutType::WAITING, 20);
	if(oneShotAction) {
		// Add code here which should only be run once
		performOneShotAction();
		oneShotAction = false;
	}
	testLock->unlockMutex();

	// Add code here which should only be run once per performOperation
	performPeriodicAction();

	// Add code here which should only be run on alternating cycles.
	if(testMode == testModes::A) {
		performActionA();
		testMode = testModes::B;
	}
	else if(testMode == testModes::B) {
		performActionB();
		testMode = testModes::A;
	}
	return result;
}

ReturnValue_t TestTask::performOneShotAction() {
	/* Everything here will only be performed once. */
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t TestTask::performPeriodicAction() {
    /* This is performed each task cycle */
	ReturnValue_t result = RETURN_OK;

	if(periodicPrinout) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::info << "TestTask::performPeriodicAction: Hello World!" << std::endl;
#else
		sif::printInfo("TestTask::performPeriodicAction: Hello World!\n");
#endif
	}
	if(periodicEvent) {
	    triggerEvent(TEST_EVENT, 0x1234, 0x4321);
	}
	return result;
}

ReturnValue_t TestTask::performActionA() {
    /* This is performed each alternating task cycle */
	ReturnValue_t result = RETURN_OK;
	return result;
}

ReturnValue_t TestTask::performActionB() {
    /* This is performed each alternating task cycle */
	ReturnValue_t result = RETURN_OK;
	return result;
}

