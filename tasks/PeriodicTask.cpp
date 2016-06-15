/**
 * \file PeriodicTask.cpp
 *
 * \brief This file contains the implementation for the PeriodicTask class.
 *
 * \author Bastian Baetz
 *
 * \date 21.07.2010
 *
 * Copyright 2010, Bastian Baetz <bastianbaetz@homail.com>
 * 	All rights reserved
 *
 */

//#include <framework/osal/object_manager.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tasks/PeriodicTask.h>

//TODO: Check if isRunning flag is useful. Shall tasks be restartable?
TaskReturn_t PeriodicTask::taskEntryPoint( TaskArgument_t argument ) {

	//The argument is re-interpreted as PeriodicTask
	PeriodicTask *originalTask( reinterpret_cast<PeriodicTask*>(argument) );
	originalTask->taskFunctionality();
	debug << "Object task " << originalTask->getId() << " returned from taskFunctionality." << std::endl;
	//TODO: destroy the task object?
}

PeriodicTask::PeriodicTask( const char *name, TaskPriority_t setPriority, size_t setStack, Interval_t setPeriod,
		void (*setDeadlineMissedFunc)(), ReturnValue_t ( *setTaskFunction )(  TaskBase* ) ) :
			TaskBase( setPriority, setStack, name ), period(setPeriod), periodId(0), deadlineMissedFunc(setDeadlineMissedFunc),
			taskFunction(setTaskFunction) {
}

PeriodicTask::~PeriodicTask() {
}

ReturnValue_t PeriodicTask::startTask() {
	debug << "PeriodicTask::startTask. TaskId: " << this->getId() << std::endl;
	this->setRunning( true );
	ReturnValue_t status;
	status = OSAL::startTask( &( this->id ), PeriodicTask::taskEntryPoint, TaskArgument_t( ( void *)this ) );
	if( status != RETURN_OK ) {
		//TODO: Call any FDIR routine?
		error << "PeriodicTask::startTask for " << this->getId() << " failed." << std::endl;
	} else {
		debug << "PeriodicTask::startTask for "   << this->getId() << " successful" << std::endl;
	}
	return status;
}

void PeriodicTask::taskFunctionality() {

	ReturnValue_t status = RETURN_OK;

	//The period is set up and started with the system call.
	status = OSAL::setAndStartPeriod( this->period, &(this->periodId) );
	if( status == RETURN_OK ) {
		//The task's "infinite" inner loop is entered.
		while( this->isRunning ) {
			//If a functionality is announced, it is started.
			if( this->taskFunction != NULL ) {
				this->taskFunction( this );
			}
			//The period is checked and restarted.
			//If the deadline was missed, the deadlineMissedFunc is called.
			if( OSAL::checkAndRestartPeriod( this->periodId, this->period ) == OSAL::TIMEOUT ) {
				error << "PeriodicTask: " << std::hex << this->getId() << " Deadline missed." << std::endl;
				if( this->deadlineMissedFunc != NULL ) {
					this->deadlineMissedFunc();
				}
			}
		}
		debug << "Returned from taskFunctionality()-Loop." << std::endl;
	} else {
		error << "PeriodicTask::setAndStartPeriod failed with status " << status << std::endl;
	}
	//Any operating system object for periodic execution is deleted.
	debug << "Deleting the PeriodicThread's period." << std::endl;
	OSAL::deletePeriod( &(this->id) );
}
