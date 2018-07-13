#include <framework/tasks/ExecutableObjectIF.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <errno.h>
#include <framework/osal/linux/PeriodicPosixTask.h>

PeriodicPosixTask::PeriodicPosixTask(const char* name_, int priority_, size_t stackSize_, uint32_t period_, void(deadlineMissedFunc_)()):PosixThread(name_,priority_,stackSize_),objectList(),started(false),periodMs(period_),deadlineMissedFunc(
		deadlineMissedFunc_) {

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
	if (newObject == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	objectList.push_back(newObject);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PeriodicPosixTask::sleepFor(uint32_t ms) {
	return PosixThread::sleep((uint64_t)ms*1000000);
}


ReturnValue_t PeriodicPosixTask::startTask(void){
	started = true;
	createTask(&taskEntryPoint,this);
	return HasReturnvaluesIF::RETURN_OK;
}

void PeriodicPosixTask::taskFunctionality(void){
	if(!started){
		suspend();
	}
	uint64_t lastWakeTime = getCurrentMonotonicTimeMs();
	//The task's "infinite" inner loop is entered.
	while (1) {
		for (ObjectList::iterator it = objectList.begin();
				it != objectList.end(); ++it) {
			(*it)->performOperation();
		}
		if(!PosixThread::delayUntil(&lastWakeTime,periodMs)){
			char name[20] = {0};
			int status = pthread_getname_np(pthread_self(),name,sizeof(name));
			if(status==0){
				error << "ObjectTask: " << name << " Deadline missed." << std::endl;
			}else{
				error << "ObjectTask: X Deadline missed. " << status << std::endl;
			}
			if (this->deadlineMissedFunc != NULL) {
				this->deadlineMissedFunc();
			}
		}
	}
}

uint32_t PeriodicPosixTask::getPeriodMs() const {
	return periodMs;
}
