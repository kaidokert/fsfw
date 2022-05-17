#include "fsfw/osal/freertos/FixedTimeslotTask.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

uint32_t FixedTimeslotTask::deadlineMissedCount = 0;
const size_t PeriodicTaskIF::MINIMUM_STACK_SIZE = configMINIMAL_STACK_SIZE;

FixedTimeslotTask::FixedTimeslotTask(TaskName name, TaskPriority setPriority,
                                     TaskStackSize setStack, TaskPeriod overallPeriod,
                                     void (*setDeadlineMissedFunc)())
    : started(false), handle(nullptr), pst(overallPeriod * 1000) {
  configSTACK_DEPTH_TYPE stackSize = setStack / sizeof(configSTACK_DEPTH_TYPE);
  xTaskCreate(taskEntryPoint, name, stackSize, this, setPriority, &handle);
  // All additional attributes are applied to the object.
  this->deadlineMissedFunc = setDeadlineMissedFunc;
}

FixedTimeslotTask::~FixedTimeslotTask() {}

void FixedTimeslotTask::taskEntryPoint(void* argument) {
  // The argument is re-interpreted as FixedTimeslotTask. The Task object is
  // global, so it is found from any place.
  FixedTimeslotTask* originalTask(reinterpret_cast<FixedTimeslotTask*>(argument));
  /* Task should not start until explicitly requested,
   * but in FreeRTOS, tasks start as soon as they are created if the scheduler
   * is running but not if the scheduler is not running.
   * To be able to accommodate both cases we check a member which is set in
   * #startTask(). If it is not set and we get here, the scheduler was started
   * before #startTask() was called and we need to suspend if it is set,
   * the scheduler was not running before #startTask() was called and we
   * can continue */

  if (not originalTask->started) {
    vTaskSuspend(NULL);
  }

  originalTask->taskFunctionality();
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "Polling task " << originalTask->handle << " returned from taskFunctionality."
             << std::endl;
#endif
}

void FixedTimeslotTask::missedDeadlineCounter() {
  FixedTimeslotTask::deadlineMissedCount++;
  if (FixedTimeslotTask::deadlineMissedCount % 10 == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "PST missed " << FixedTimeslotTask::deadlineMissedCount << " deadlines."
               << std::endl;
#endif
  }
}

ReturnValue_t FixedTimeslotTask::startTask() {
  started = true;

  // We must not call resume if scheduler is not started yet
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    vTaskResume(handle);
  }

  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t FixedTimeslotTask::addSlot(object_id_t componentId, uint32_t slotTimeMs,
                                         int8_t executionStep) {
  ExecutableObjectIF* handler = ObjectManager::instance()->get<ExecutableObjectIF>(componentId);
  if (handler != nullptr) {
    pst.addSlot(componentId, slotTimeMs, executionStep, handler, this);
    return HasReturnvaluesIF::RETURN_OK;
  }

#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::error << "Component " << std::hex << componentId << " not found, not adding it to pst"
             << std::endl;
#endif
  return HasReturnvaluesIF::RETURN_FAILED;
}

uint32_t FixedTimeslotTask::getPeriodMs() const { return pst.getLengthMs(); }

ReturnValue_t FixedTimeslotTask::checkSequence() const { return pst.checkSequence(); }

void FixedTimeslotTask::taskFunctionality() {
  // A local iterator for the Polling Sequence Table is created to find the
  // start time for the first entry.
  auto slotListIter = pst.current;

  pst.intializeSequenceAfterTaskCreation();

  // The start time for the first entry is read.
  uint32_t intervalMs = slotListIter->pollingTimeMs;
  TickType_t interval = pdMS_TO_TICKS(intervalMs);

  TickType_t xLastWakeTime;
  /* The xLastWakeTime variable needs to be initialized with the current tick
   count. Note that this is the only time the variable is written to
   explicitly. After this assignment, xLastWakeTime is updated automatically
   internally within vTaskDelayUntil(). */
  xLastWakeTime = xTaskGetTickCount();

  // wait for first entry's start time
  if (interval > 0) {
    vTaskDelayUntil(&xLastWakeTime, interval);
  }

  /* Enter the loop that defines the task behavior. */
  for (;;) {
    // The component for this slot is executed and the next one is chosen.
    this->pst.executeAndAdvance();
    if (not pst.slotFollowsImmediately()) {
      // Get the interval till execution of the next slot.
      intervalMs = this->pst.getIntervalToPreviousSlotMs();
      interval = pdMS_TO_TICKS(intervalMs);

#if (tskKERNEL_VERSION_MAJOR == 10 && tskKERNEL_VERSION_MINOR >= 4) || tskKERNEL_VERSION_MAJOR > 10
      BaseType_t wasDelayed = xTaskDelayUntil(&xLastWakeTime, interval);
      if (wasDelayed == pdFALSE) {
        handleMissedDeadline();
      }
#else
      if (checkMissedDeadline(xLastWakeTime, interval)) {
        handleMissedDeadline();
      }
      // Wait for the interval. This exits immediately if a deadline was
      // missed while also updating the last wake time.
      vTaskDelayUntil(&xLastWakeTime, interval);
#endif
    }
  }
}

void FixedTimeslotTask::handleMissedDeadline() {
  if (deadlineMissedFunc != nullptr) {
    this->deadlineMissedFunc();
  }
}

ReturnValue_t FixedTimeslotTask::sleepFor(uint32_t ms) {
  vTaskDelay(pdMS_TO_TICKS(ms));
  return HasReturnvaluesIF::RETURN_OK;
}

TaskHandle_t FixedTimeslotTask::getTaskHandle() { return handle; }
