/**
 * \file OPUSPollingTask.cpp
 *
 * \brief This file contains the implementation for the OPUSPollingTask class.
 *
 * \author Bastian Baetz
 *
 * \date 17.03.2011
 *
 */

#include <framework/devicehandlers/PollingTask.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

uint32_t PollingTask::deadlineMissedCount = 0;

PollingTask::PollingTask( const char *name, TaskPriority_t setPriority, size_t setStack, void (*setDeadlineMissedFunc)(), object_id_t getPst ) :
	TaskBase( setPriority, setStack, name ), periodId(0) {
	// All additional attributes are applied to the object.
	this->deadlineMissedFunc = setDeadlineMissedFunc;
	this->pst = objectManager->get<PollingSequence>( getPst );
}

PollingTask::~PollingTask() {
	// The PollingSequence destructor is called, if a sequence is announced.
	if ( this->pst != NULL ) {
		this->pst->~PollingSequence();
	} else {
		// There was no memory allocation, so there's nothing to do.
	}
}

TaskReturn_t PollingTask::taskEntryPoint( TaskArgument_t argument ) {

	//The argument is re-interpreted as PollingTask.
	PollingTask *originalTask( reinterpret_cast<PollingTask*>( argument ) );
	if ( originalTask->pst != NULL ) {
		//The task's functionality is called.
		originalTask->taskFunctionality();
	} else {

	}
	debug << "Polling task " << originalTask->getId() << " returned from taskFunctionality." << std::endl;
}

void PollingTask::missedDeadlineCounter() {
	PollingTask::deadlineMissedCount++;
	if (PollingTask::deadlineMissedCount%10 == 0) {
		error << "PST missed " << PollingTask::deadlineMissedCount << " deadlines." << std::endl;
	}
}

ReturnValue_t PollingTask::startTask() {
	this->setRunning( true );
	ReturnValue_t status;
	status = OSAL::startTask( &( this->id ), PollingTask::taskEntryPoint, TaskArgument_t( ( void *)this ) );
	if( status != RETURN_OK ) {
		//TODO: Call any FDIR routine?
		error << "PollingTask::startTask for " << std::hex << this->getId() << std::dec << " failed." << std::endl;
	} else {
//		debug << "PollingTask::startTask for "    << std::hex << this->getId() << std::dec << " successful" << std::endl;
	}
	return status;
}

void PollingTask::taskFunctionality() {
	ReturnValue_t	status = OSAL::TIMEOUT;
	Interval_t		interval;
	// A local iterator for the Polling Sequence Table is created to find the start time for the first entry.
	std::list<PollingSlot*>::iterator it;

	it = this->pst->current;
	//The start time for the first entry is read.
	interval = ( *it )->pollingTime;
	//The period is set up and started with the system call.
	status = OSAL::setAndStartPeriod( interval, &( this->periodId ) );
	if( status == RETURN_OK ) {
		//The task's "infinite" inner loop is entered.
		while( this->isRunning ) {
			if ( pst->slotFollowsImmediately() ) {
				//Do nothing
			} else {
				//The interval for the next polling slot is selected.
				interval = this->pst->getInterval();
				//The period is checked and restarted with the new interval.
				//If the deadline was missed, the deadlineMissedFunc is called.
				status = OSAL::checkAndRestartPeriod( this->periodId, interval );
				if( status == OSAL::TIMEOUT ) {
					if( this->deadlineMissedFunc != NULL ) {
						this->deadlineMissedFunc();
					}
				}
			}
			//The device handler for this slot is executed and the next one is chosen.
			this->pst->pollAndAdvance();
		}
	} else {
		error << "PollingTask::setAndStartPeriod failed with status "<< status << std::endl;
	}
	//Any operating system object for periodic execution is deleted.
	debug << "Deleting the PollingTask's period." << std::endl;
	OSAL::deletePeriod( &(this->id) );
}
