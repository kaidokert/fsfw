#include "fsfw/osal/rtems/FixedTimeslotTask.h"

#include <rtems/io.h>
#include <rtems/rtems/status.h>
#include <rtems/rtems/tasks.h>
#include <rtems/rtems/types.h>

#include "fsfw/osal/rtems/RtemsBasic.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

#if FSFW_CPP_OSTREAM_ENABLED == 1
#include <iostream>
#endif

#include <cstddef>

FixedTimeslotTask::FixedTimeslotTask(const char *name, rtems_task_priority setPriority,
                                     size_t setStack, TaskPeriod setOverallPeriod,
                                     TaskDeadlineMissedFunction dlmFunc_)
    : FixedTimeslotTaskBase(setOverallPeriod, dlmFunc_),
      RTEMSTaskBase(setPriority, setStack, name),
      periodId(0) {}

FixedTimeslotTask::~FixedTimeslotTask() = default;

rtems_task FixedTimeslotTask::taskEntryPoint(rtems_task_argument argument) {
  /* The argument is re-interpreted as a FixedTimeslotTask */
  auto *originalTask(reinterpret_cast<FixedTimeslotTask *>(argument));
  /* The task's functionality is called. */
  return originalTask->taskFunctionality();
  /* Should never be reached */
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::error << "Polling task " << originalTask->getId() << " returned from taskFunctionality."
             << std::endl;
#endif
}

ReturnValue_t FixedTimeslotTask::startTask() {
  rtems_status_code status =
      rtems_task_start(id, FixedTimeslotTask::taskEntryPoint, rtems_task_argument((void *)this));
  if (status != RTEMS_SUCCESSFUL) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "PollingTask::startTask for " << std::hex << this->getId() << std::dec
               << " failed." << std::endl;
#endif
  }
  switch (status) {
    case RTEMS_SUCCESSFUL:
      // ask started successfully
      return returnvalue::OK;
    default:
      /*
          RTEMS_INVALID_ADDRESS - invalid task entry point
              RTEMS_INVALID_ID - invalid task id
              RTEMS_INCORRECT_STATE - task not in the dormant state
              RTEMS_ILLEGAL_ON_REMOTE_OBJECT - cannot start remote task */
      return returnvalue::FAILED;
  }
}

[[noreturn]] void FixedTimeslotTask::taskFunctionality() {
  /* A local iterator for the Polling Sequence Table is created to find the start time for
      the first entry. */
  auto it = pollingSeqTable.current;

  /* Initialize the PST with the correct calling task */
  pollingSeqTable.intializeSequenceAfterTaskCreation();

  /* The start time for the first entry is read. */
  rtems_interval interval = RtemsBasic::convertMsToTicks(it->pollingTimeMs);
  RTEMSTaskBase::setAndStartPeriod(interval, &periodId);
  // The task's "infinite" inner loop is entered.
  while (true) {
    if (pollingSeqTable.slotFollowsImmediately()) {
      /* Do nothing */
    } else {
      /* The interval for the next polling slot is selected. */
      interval = RtemsBasic::convertMsToTicks(pollingSeqTable.getIntervalToNextSlotMs());
      /* The period is checked and restarted with the new interval.
                  If the deadline was missed, the deadlineMissedFunc is called. */
      rtems_status_code status = RTEMSTaskBase::restartPeriod(interval, periodId);
      if (status == RTEMS_TIMEOUT) {
        if (dlmFunc != nullptr) {
          dlmFunc();
        }
      }
    }
    /* The device handler for this slot is executed and the next one is chosen. */
    this->pollingSeqTable.executeAndAdvance();
  }
}

ReturnValue_t FixedTimeslotTask::sleepFor(uint32_t ms) { return RTEMSTaskBase::sleepFor(ms); };
