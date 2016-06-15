/*
 * TaskBase.cpp
 *
 *  Created on: Nov 5, 2012
 *      Author: baetz
 */




#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tasks/TaskBase.h>

TaskBase::TaskBase( TaskPriority_t set_priority, size_t stack_size , const char  *name) : isRunning(false) {
	Name_t osalName = 0;
	for (uint8_t i = 0; i < 4; i++){
		if (name[i] == 0){
			break;
		}
		osalName += name[i] << (8*(3-i));
	}
	//The task is created with the operating system's system call.
	ReturnValue_t status = OSAL::createTask(
		osalName, set_priority,
		stack_size,
		OSAL::PREEMPT | OSAL::NO_TIMESLICE | OSAL::NO_ASR, OSAL::FLOATING_POINT,
		&( this->id )
	);
	//TODO: Safe system halt or FDIR call on failed task creation?
	if( status != RETURN_OK ) {
		error << "TaskBase::TaskBase: createTask with name " << std::hex << osalName << std::dec << " failed with return code " << (uint32_t)status << std::endl;
		this->id = 0;
	} else {
//		debug << "TaskBase::TaskBase: createTask successful. Name: " << std::hex << new_name << ", ID: " << this->id << std::dec << std::endl;
	}
}

TaskBase::~TaskBase() {
	OSAL::deleteTask( &(this->id) );
}

TaskId_t TaskBase::getId() {
	return this->id;
}

void TaskBase::setRunning( bool set ) {
	this->isRunning = set;
}

uint8_t TaskBase::taskCounter = 0;
