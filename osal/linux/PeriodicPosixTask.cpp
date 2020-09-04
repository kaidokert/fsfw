#include "../../tasks/ExecutableObjectIF.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"
#include <errno.h>
#include "PeriodicPosixTask.h"

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
	ExecutableObjectIF* newObject = objectManager->get<ExecutableObjectIF>(
			object);
	if (newObject == nullptr) {
		sif::error << "PeriodicTask::addComponent: Invalid object. Make sure"
				<< " it implements ExecutableObjectIF!" << std::endl;
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
	//sif::info << stackSize << std::endl;
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
			char name[20] = {0};
			int status = pthread_getname_np(pthread_self(), name, sizeof(name));
			if(status == 0) {
				sif::error << "PeriodicPosixTask " << name << ": Deadline "
						"missed." << std::endl;
			}
			else {
				sif::error << "PeriodicPosixTask X: Deadline missed. " <<
						status << std::endl;
			}
			if (this->deadlineMissedFunc != nullptr) {
				this->deadlineMissedFunc();
			}
		}
	}
}

uint32_t PeriodicPosixTask::getPeriodMs() const {
	return periodMs;
}
