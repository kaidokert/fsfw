#include "fsfw/osal/freertos/PeriodicTask.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

PeriodicTask::PeriodicTask(const char* name, TaskPriority setPriority, TaskStackSize setStack,
                           TaskPeriod setPeriod, TaskDeadlineMissedFunction dlmFunc_)
    : PeriodicTaskBase(setPeriod, dlmFunc_), started(false), handle(nullptr) {
  configSTACK_DEPTH_TYPE stackSize = setStack / sizeof(configSTACK_DEPTH_TYPE);
  BaseType_t status = xTaskCreate(taskEntryPoint, name, stackSize, this, setPriority, &handle);
  if (status != pdPASS) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "PeriodicTask::PeriodicTask Insufficient heap memory remaining. Status: "
               << status << std::endl;
#else
    sif::printDebug("PeriodicTask::PeriodicTask: Insufficient heap memory remaining. Status: %d\n",
                    status);
#endif
  }
}

// Do not delete objects, we were responsible for ptrs only.
PeriodicTask::~PeriodicTask() = default;

void PeriodicTask::taskEntryPoint(void* argument) {
  // The argument is re-interpreted as PeriodicTask. The Task object is
  // global, so it is found from any place.
  auto* originalTask(reinterpret_cast<PeriodicTask*>(argument));
  /* Task should not start until explicitly requested,
   * but in FreeRTOS, tasks start as soon as they are created if the scheduler
   * is running but not if the scheduler is not running.
   * To be able to accommodate both cases we check a member which is set in
   * #startTask(). If it is not set and we get here, the scheduler was started
   * before #startTask() was called and we need to suspend if it is set,
   * the scheduler was not running before #startTask() was called and we
   * can continue */

  if (not originalTask->started) {
    vTaskSuspend(nullptr);
  }

  originalTask->taskFunctionality();
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "Polling task " << originalTask->handle << " returned from taskFunctionality."
             << std::endl;
#endif
}

ReturnValue_t PeriodicTask::startTask() {
  started = true;

  // We must not call resume if scheduler is not started yet
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    vTaskResume(handle);
  }

  return returnvalue::OK;
}

ReturnValue_t PeriodicTask::sleepFor(uint32_t ms) {
  vTaskDelay(pdMS_TO_TICKS(ms));
  return returnvalue::OK;
}

[[noreturn]] void PeriodicTask::taskFunctionality() {
  TickType_t xLastWakeTime;
  const TickType_t xPeriod = pdMS_TO_TICKS(this->period * 1000.);

  initObjsAfterTaskCreation();

  /* The xLastWakeTime variable needs to be initialized with the current tick
   count. Note that this is the only time the variable is written to
   explicitly. After this assignment, xLastWakeTime is updated automatically
   internally within vTaskDelayUntil(). */
  xLastWakeTime = xTaskGetTickCount();
  /* Enter the loop that defines the task behavior. */
  for (;;) {
    for (auto const& objectPair : objectList) {
      objectPair.first->performOperation(objectPair.second);
    }

#if (tskKERNEL_VERSION_MAJOR == 10 && tskKERNEL_VERSION_MINOR >= 4) || tskKERNEL_VERSION_MAJOR > 10
    BaseType_t wasDelayed = xTaskDelayUntil(&xLastWakeTime, xPeriod);
    if (wasDelayed == pdFALSE) {
      handleMissedDeadline();
    }
#else
    if (checkMissedDeadline(xLastWakeTime, xPeriod)) {
      handleMissedDeadline();
    }
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
#endif
  }
}

TaskHandle_t PeriodicTask::getTaskHandle() { return handle; }

void PeriodicTask::handleMissedDeadline() {
  if (dlmFunc != nullptr) {
    dlmFunc();
  }
}
