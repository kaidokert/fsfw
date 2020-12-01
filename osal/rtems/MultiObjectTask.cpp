/**
 * @file	MultiObjectTask.cpp
 * @brief	This file defines the MultiObjectTask class.
 * @date	30.01.2014
 * @author	baetz
 */

#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../tasks/ExecutableObjectIF.h"
#include "MultiObjectTask.h"

MultiObjectTask::MultiObjectTask(const char *name, rtems_task_priority setPriority,
		size_t setStack, rtems_interval setPeriod, void (*setDeadlineMissedFunc)()) :
		TaskBase(setPriority, setStack, name), periodTicks(
				RtemsBasic::convertMsToTicks(setPeriod)), periodId(0), deadlineMissedFunc(
				setDeadlineMissedFunc) {
}

MultiObjectTask::~MultiObjectTask(void) {
	//Do not delete objects, we were responsible for ptrs only.
	rtems_rate_monotonic_delete(periodId);
}
rtems_task MultiObjectTask::taskEntryPoint(rtems_task_argument argument) {
	//The argument is re-interpreted as MultiObjectTask. The Task object is global, so it is found from any place.
	MultiObjectTask *originalTask(reinterpret_cast<MultiObjectTask*>(argument));
	originalTask->taskFunctionality();
}

ReturnValue_t MultiObjectTask::startTask() {
	rtems_status_code status = rtems_task_start(id, MultiObjectTask::taskEntryPoint,
			rtems_task_argument((void *) this));
	if (status != RTEMS_SUCCESSFUL) {
		sif::error << "ObjectTask::startTask for " << std::hex << this->getId()
				<< std::dec << " failed." << std::endl;
	}
	switch(status){
	case RTEMS_SUCCESSFUL:
		//ask started successfully
		return HasReturnvaluesIF::RETURN_OK;
	default:
/*		RTEMS_INVALID_ADDRESS - invalid task entry point
		RTEMS_INVALID_ID - invalid task id
		RTEMS_INCORRECT_STATE - task not in the dormant state
		RTEMS_ILLEGAL_ON_REMOTE_OBJECT - cannot start remote task */
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t MultiObjectTask::sleepFor(uint32_t ms) {
	return TaskBase::sleepFor(ms);
}

void MultiObjectTask::taskFunctionality() {
	TaskBase::setAndStartPeriod(periodTicks,&periodId);
	//The task's "infinite" inner loop is entered.
	while (1) {
		for (ObjectList::iterator it = objectList.begin();
				it != objectList.end(); ++it) {
			(*it)->performOperation();
		}
		rtems_status_code status = TaskBase::restartPeriod(periodTicks,periodId);
		if (status == RTEMS_TIMEOUT) {
			char nameSpace[8] = { 0 };
			char* ptr = rtems_object_get_name(getId(), sizeof(nameSpace),
					nameSpace);
			sif::error << "ObjectTask: " << ptr << " Deadline missed." << std::endl;
			if (this->deadlineMissedFunc != nullptr) {
				this->deadlineMissedFunc();
			}
		}
	}
}

ReturnValue_t MultiObjectTask::addComponent(object_id_t object) {
	ExecutableObjectIF* newObject = objectManager->get<ExecutableObjectIF>(
			object);
	if (newObject == nullptr) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	objectList.push_back(newObject);
	newObject->setTaskIF(this);

	return HasReturnvaluesIF::RETURN_OK;
}

uint32_t MultiObjectTask::getPeriodMs() const {
	return RtemsBasic::convertTicksToMs(periodTicks);
}
