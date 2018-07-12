/**
 * @file	MultiObjectTask.cpp
 * @brief	This file defines the MultiObjectTask class.
 * @date	30.01.2014
 * @author	baetz
 */

#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tasks/ExecutableObjectIF.h>
#include "MultiObjectTask.h"

MultiObjectTask::MultiObjectTask(const char *name, rtems_task_priority setPriority,
		size_t setStack, rtems_interval setPeriod, void (*setDeadlineMissedFunc)()) :
		TaskBase(setPriority, setStack, name), periodTicks(
				RtemsBasic::convertMsToTicks(setPeriod)), periodId(0), deadlineMissedFunc(
				setDeadlineMissedFunc) {
	rtems_name periodName = (('P' << 24) + ('e' << 16) + ('r' << 8) + 'd');
	rtems_status_code status = rtems_rate_monotonic_create(periodName,
			&periodId);
	if (status != RTEMS_SUCCESSFUL) {
		error << "ObjectTask::period create failed with status " << status
				<< std::endl;
	}
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
		error << "ObjectTask::startTask for " << std::hex << this->getId()
				<< std::dec << " failed." << std::endl;
	}
	return RtemsBasic::convertReturnCode(status);
}

ReturnValue_t MultiObjectTask::sleepFor(uint32_t ms) {
	return TaskBase::sleepFor(ms);
}

void MultiObjectTask::taskFunctionality() {
	//The +1 is necessary to avoid a call with period = 0, which does not start the period.
	rtems_status_code status = rtems_rate_monotonic_period(periodId,
			periodTicks + 1);
	if (status != RTEMS_SUCCESSFUL) {
		error << "ObjectTask::period start failed with status " << status
				<< std::endl;
		return;
	}
	//The task's "infinite" inner loop is entered.
	while (1) {
		for (ObjectList::iterator it = objectList.begin();
				it != objectList.end(); ++it) {
			(*it)->performOperation();
		}
		status = rtems_rate_monotonic_period(periodId, periodTicks + 1);
		if (status == RTEMS_TIMEOUT) {
			char nameSpace[8] = { 0 };
			char* ptr = rtems_object_get_name(getId(), sizeof(nameSpace),
					nameSpace);
			error << "ObjectTask: " << ptr << " Deadline missed." << std::endl;
			if (this->deadlineMissedFunc != NULL) {
				this->deadlineMissedFunc();
			}
		}
	}
}

ReturnValue_t MultiObjectTask::addComponent(object_id_t object) {
	ExecutableObjectIF* newObject = objectManager->get<ExecutableObjectIF>(
			object);
	if (newObject == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	objectList.push_back(newObject);
	return HasReturnvaluesIF::RETURN_OK;
}

uint32_t MultiObjectTask::getPeriodMs() const {
	return RtemsBasic::convertTicksToMs(periodTicks);
}
