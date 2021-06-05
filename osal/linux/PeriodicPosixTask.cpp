#include "PeriodicPosixTask.h"

#include "../../objectmanager/ObjectManager.h"
#include "../../tasks/ExecutableObjectIF.h"
#include "../../serviceinterface/ServiceInterface.h"

#include <errno.h>

PeriodicPosixTask::PeriodicPosixTask(const char* name_, int priority_,
		size_t stackSize_, uint32_t period_, void(deadlineMissedFunc_)()):
		PosixThread(name_, priority_, stackSize_), objectList(), started(false),
		periodMs(period_), deadlineMissedFunc(deadlineMissedFunc_) {
}

PeriodicPosixTask::~PeriodicPosixTask() {
	//Not Implemented
}

void* PeriodicPosixTask::taskEntryPoint(void* arg) {
	//The argument is re-interpreted as PollingTask.
	PeriodicPosixTask *originalTask(reinterpret_cast<PeriodicPosixTask*>(arg));
	//The task's functionality is called.
	originalTask->taskFunctionality();
	return NULL;
}

ReturnValue_t PeriodicPosixTask::addComponent(object_id_t object) {
	ExecutableObjectIF* newObject = ObjectManager::instance()->get<ExecutableObjectIF>(
			object);
	if (newObject == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "PeriodicTask::addComponent: Invalid object. Make sure"
				<< " it implements ExecutableObjectIF!" << std::endl;
#else
		sif::printError("PeriodicTask::addComponent: Invalid object. Make sure it"
		        "implements ExecutableObjectIF!\n");
#endif
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	objectList.push_back(newObject);
	newObject->setTaskIF(this);

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PeriodicPosixTask::sleepFor(uint32_t ms) {
	return PosixThread::sleep((uint64_t)ms*1000000);
}


ReturnValue_t PeriodicPosixTask::startTask(void) {
	started = true;
	PosixThread::createTask(&taskEntryPoint,this);
	return HasReturnvaluesIF::RETURN_OK;
}

void PeriodicPosixTask::taskFunctionality(void) {
	if(not started) {
		suspend();
	}

	for (auto const &object: objectList) {
		object->initializeAfterTaskCreation();
	}

	uint64_t lastWakeTime = getCurrentMonotonicTimeMs();
	//The task's "infinite" inner loop is entered.
	while (1) {
		for (auto const &object: objectList) {
			object->performOperation();
		}

		if(not PosixThread::delayUntil(&lastWakeTime, periodMs)){
			if (this->deadlineMissedFunc != nullptr) {
				this->deadlineMissedFunc();
			}
		}
	}
}

uint32_t PeriodicPosixTask::getPeriodMs() const {
	return periodMs;
}
