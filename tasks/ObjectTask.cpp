/*
 * ObjectTask.cpp
 *
 *  Created on: 03.02.2012
 *      Author: baetz
 */

#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tasks/ObjectTask.h>

ObjectTask::ObjectTask( const char *name, TaskPriority_t setPriority, size_t setStack, Interval_t setPeriod, void (*setDeadlineMissedFunc)(), ExecutableObjectIF* (*initFunction)() ) :
	TaskBase( setPriority, setStack, name ), period(setPeriod), periodId(0), deadlineMissedFunc(setDeadlineMissedFunc) {
	// All additional attributes are applied to the object.
	this->executingObject = initFunction();

}

ObjectTask::ObjectTask(const char *name, TaskPriority_t setPriority, size_t setStack,
		Interval_t setPeriod, void (*setDeadlineMissedFunc)(),
		object_id_t object_id) : TaskBase( setPriority, setStack, name ), period(setPeriod), periodId(0), deadlineMissedFunc(setDeadlineMissedFunc) {
	this->executingObject = objectManager->get<ExecutableObjectIF>( object_id );
}

ObjectTask::~ObjectTask() {
}

TaskReturn_t ObjectTask::taskEntryPoint( TaskArgument_t argument ) {
	//The argument is re-interpreted as ObjectTask. The Task object is global, so it is found from any place.
	ObjectTask *originalTask( reinterpret_cast<ObjectTask*>( argument ) );
	originalTask->taskFunctionality();
	debug << "Object task " << originalTask->getId() << " returned from taskFunctionality. Deleting task." << std::endl;
	//TODO: destroy the task object?
}

ReturnValue_t ObjectTask::startTask() {
	this->setRunning( true );
	ReturnValue_t status;
	status = OSAL::startTask( &( this->id ), ObjectTask::taskEntryPoint, TaskArgument_t( ( void *)this ) );
	if( status != RETURN_OK ) {
		//TODO: Call any FDIR routine?
		error << "ObjectTask::startTask for "  << std::hex << this->getId() << std::dec << " failed." << std::endl;
	} else {
//		debug << "ObjectTask::startTask for "    << std::hex << this->getId() << std::dec << " successful" << std::endl;
	}
	return status;
}

void ObjectTask::taskFunctionality() {
	ReturnValue_t status = OSAL::TIMEOUT;
	//The period is set up and started with the system call.
	if ( this->executingObject != NULL ) {
		status = OSAL::setAndStartPeriod( this->period, &(this->periodId) );
		if( status == RETURN_OK ) {
			//The task's "infinite" inner loop is entered.
			while( this->isRunning ) {

				this->executingObject->performOperation();

				if( OSAL::checkAndRestartPeriod( this->periodId, this->period ) == OSAL::TIMEOUT ) {
					char nameSpace[8] = {0};
					char* ptr = rtems_object_get_name(getId(), sizeof(nameSpace), nameSpace);
					error << "ObjectTask: " << ptr << " Deadline missed." << std::endl;
					if( this->deadlineMissedFunc != NULL ) {
						this->deadlineMissedFunc();
					}
				}
			}
			debug << "Returned from taskFunctionality()-Loop." << std::endl;
		} else {
			error << "ObjectTask::setAndStartPeriod failed with status " << status << std::endl;
		}
	} else {
		error << "ObjectTask::taskFunctionality. Object not found." << std::endl;
	}
	//Any operating system object for periodic execution is deleted.
	debug << "Deleting the ObjectTask's period." << std::endl;
	OSAL::deletePeriod( &(this->id) );
}
