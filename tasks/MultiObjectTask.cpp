/**
 * @file	MultiObjectTask.cpp
 * @brief	This file defines the MultiObjectTask class.
 * @date	30.01.2014
 * @author	baetz
 */

#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tasks/MultiObjectTask.h>

MultiObjectTask::MultiObjectTask(const char *name, TaskPriority_t setPriority,
		size_t setStack, Interval_t setPeriod, void (*setDeadlineMissedFunc)()) :
		TaskBase(setPriority, setStack, name), period(setPeriod), periodId(0), deadlineMissedFunc(
				setDeadlineMissedFunc) {

}

MultiObjectTask::~MultiObjectTask(void) {
	//Do not delete objects!
}
TaskReturn_t MultiObjectTask::taskEntryPoint(TaskArgument_t argument) {
	//The argument is re-interpreted as ObjectTask. The Task object is global, so it is found from any place.
	MultiObjectTask *originalTask(reinterpret_cast<MultiObjectTask*>(argument));
	originalTask->taskFunctionality();
	debug << "MultiObjectTask " << originalTask->getId()
			<< " returned from taskFunctionality. Deleting task." << std::endl;
}

ReturnValue_t MultiObjectTask::startTask() {
	this->setRunning( true);
	ReturnValue_t status;
	status = OSAL::startTask(&(this->id), MultiObjectTask::taskEntryPoint,
			TaskArgument_t((void *) this));
	if (status != RETURN_OK) {
		//TODO: Call any FDIR routine?
		error << "MultiObjectTask::startTask for " << std::hex << this->getId()
				<< std::dec << " failed." << std::endl;
	} else {
//		debug << "ObjectTask::startTask for "    << std::hex << this->getId() << std::dec << " successful" << std::endl;
	}
	return status;
}

void MultiObjectTask::taskFunctionality() {
	ReturnValue_t status = OSAL::TIMEOUT;
	//The period is set up and started with the system call.
	if (objectList.begin() != objectList.end()) {
		status = OSAL::setAndStartPeriod(this->period, &(this->periodId));
		if (status == RETURN_OK) {
			//The task's "infinite" inner loop is entered.
			while (this->isRunning) {
				for (ObjectList::iterator it = objectList.begin();
						it != objectList.end(); ++it) {
					(*it)->performOperation();
				}
				if (OSAL::checkAndRestartPeriod(this->periodId, this->period)
						== OSAL::TIMEOUT) {
					char nameSpace[8] = { 0 };
					char* ptr = rtems_object_get_name(getId(),
							sizeof(nameSpace), nameSpace);
					error << "MultiObjectTask: " << ptr << " Deadline missed."
							<< std::endl;
					if (this->deadlineMissedFunc != NULL) {
						this->deadlineMissedFunc();
					}
				}
			}
			debug << "MultiObjectTask: Returned from taskFunctionality()-Loop."
					<< std::endl;
		} else {
			error << "MultiObjectTask::setAndStartPeriod failed with status "
					<< status << std::endl;
		}
	} else {
		error << "MultiObjectTask::taskFunctionality. No object assigned."
				<< std::endl;
	}
	//Any operating system object for periodic execution is deleted.
	debug << "Deleting the ObjectTask's period." << std::endl;
	OSAL::deletePeriod(&(this->id));
}

ReturnValue_t MultiObjectTask::addObject(object_id_t object) {
	ExecutableObjectIF* newObject = objectManager->get<ExecutableObjectIF>(
			object);
	if (newObject == NULL) {
		return RETURN_FAILED;
	}
	objectList.push_back(newObject);
	return RETURN_OK;
}

ReturnValue_t MultiObjectTask::addObject(ExecutableObjectIF* object) {
	if (object == NULL) {
		return RETURN_FAILED;
	}
	objectList.push_back(object);
	return RETURN_OK;
}
