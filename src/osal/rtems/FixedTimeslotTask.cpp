#include "fsfw/osal/rtems/FixedTimeslotTask.h"
#include "fsfw/osal/rtems/RtemsBasic.h"

#include "fsfw/tasks/FixedSequenceSlot.h"
#include "fsfw/objectmanager/SystemObjectIF.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

#include <rtems/bspIo.h>
#include <rtems/io.h>
#include <rtems/rtems/ratemon.h>
#include <rtems/rtems/status.h>
#include <rtems/rtems/tasks.h>
#include <rtems/rtems/types.h>
#include <sys/_stdint.h>

#if FSFW_CPP_OSTREAM_ENABLED == 1
#include <iostream>
#endif

#include <cstddef>
#include <list>

uint32_t FixedTimeslotTask::deadlineMissedCount = 0;

FixedTimeslotTask::FixedTimeslotTask(const char *name, rtems_task_priority setPriority,
        size_t setStack, uint32_t setOverallPeriod, void (*setDeadlineMissedFunc)(void)):
		RTEMSTaskBase(setPriority, setStack, name), periodId(0), pst(setOverallPeriod) {
    // All additional attributes are applied to the object.
    this->deadlineMissedFunc = setDeadlineMissedFunc;
}

FixedTimeslotTask::~FixedTimeslotTask() {
}

rtems_task FixedTimeslotTask::taskEntryPoint(rtems_task_argument argument) {
    /* The argument is re-interpreted as a FixedTimeslotTask */
    FixedTimeslotTask *originalTask(reinterpret_cast<FixedTimeslotTask*>(argument));
    /* The task's functionality is called. */
    return originalTask->taskFunctionality();
    /* Should never be reached */
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Polling task " << originalTask->getId() << " returned from taskFunctionality." <<
            std::endl;
#endif
}

void FixedTimeslotTask::missedDeadlineCounter() {
    FixedTimeslotTask::deadlineMissedCount++;
    if (FixedTimeslotTask::deadlineMissedCount % 10 == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "PST missed " << FixedTimeslotTask::deadlineMissedCount
                << " deadlines." << std::endl;
#endif
    }
}

ReturnValue_t FixedTimeslotTask::startTask() {
    rtems_status_code status = rtems_task_start(id, FixedTimeslotTask::taskEntryPoint,
            rtems_task_argument((void *) this));
    if (status != RTEMS_SUCCESSFUL) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "PollingTask::startTask for " << std::hex << this->getId()
				        << std::dec << " failed." << std::endl;
#endif
    }
    switch(status){
    case RTEMS_SUCCESSFUL:
        //ask started successfully
        return HasReturnvaluesIF::RETURN_OK;
    default:
        /*
	    RTEMS_INVALID_ADDRESS - invalid task entry point
		RTEMS_INVALID_ID - invalid task id
		RTEMS_INCORRECT_STATE - task not in the dormant state
		RTEMS_ILLEGAL_ON_REMOTE_OBJECT - cannot start remote task */
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

ReturnValue_t FixedTimeslotTask::addSlot(object_id_t componentId,
        uint32_t slotTimeMs, int8_t executionStep) {
    ExecutableObjectIF* object = ObjectManager::instance()->get<ExecutableObjectIF>(componentId);
    if (object != nullptr) {
        pst.addSlot(componentId, slotTimeMs, executionStep, object, this);
        return HasReturnvaluesIF::RETURN_OK;
    }

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Component " << std::hex << componentId <<
            " not found, not adding it to pst" << std::endl;
#endif
    return HasReturnvaluesIF::RETURN_FAILED;
}

uint32_t FixedTimeslotTask::getPeriodMs() const {
    return pst.getLengthMs();
}

ReturnValue_t FixedTimeslotTask::checkSequence() const {
    return pst.checkSequence();
}

void FixedTimeslotTask::taskFunctionality() {
    /* A local iterator for the Polling Sequence Table is created to find the start time for
	the first entry. */
    FixedSlotSequence::SlotListIter it = pst.current;

    /* Initialize the PST with the correct calling task */
    pst.intializeSequenceAfterTaskCreation();

    /* The start time for the first entry is read. */
    rtems_interval interval = RtemsBasic::convertMsToTicks(it->pollingTimeMs);
    RTEMSTaskBase::setAndStartPeriod(interval,&periodId);
    //The task's "infinite" inner loop is entered.
    while (1) {
        if (pst.slotFollowsImmediately()) {
            /* Do nothing */
        }
        else {
            /* The interval for the next polling slot is selected. */
            interval = RtemsBasic::convertMsToTicks(this->pst.getIntervalToNextSlotMs());
            /* The period is checked and restarted with the new interval.
			If the deadline was missed, the deadlineMissedFunc is called. */
            rtems_status_code status = RTEMSTaskBase::restartPeriod(interval,periodId);
            if (status == RTEMS_TIMEOUT) {
                if (this->deadlineMissedFunc != nullptr) {
                    this->deadlineMissedFunc();
                }
            }
        }
        /* The device handler for this slot is executed and the next one is chosen. */
        this->pst.executeAndAdvance();
    }
}

ReturnValue_t FixedTimeslotTask::sleepFor(uint32_t ms){
    return RTEMSTaskBase::sleepFor(ms);
};
