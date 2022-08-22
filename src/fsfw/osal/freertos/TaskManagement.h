#ifndef FSFW_OSAL_FREERTOS_TASKMANAGEMENT_H_
#define FSFW_OSAL_FREERTOS_TASKMANAGEMENT_H_

#include <cstdint>

#include "../../returnvalues/returnvalue.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * Architecture dependant portmacro.h function call.
 * Should be implemented in bsp.
 */
extern "C" void vRequestContextSwitchFromISR();

/*!
 * Used by functions to tell if they are being called from
 * within an ISR or from a regular task. This is required because FreeRTOS
 * has different functions for handling semaphores and messages from within
 * an ISR and task.
 */
enum class CallContext {
  TASK = 0x00,  //!< task_context
  ISR = 0xFF    //!< isr_context
};

namespace TaskManagement {
/**
 * @brief	In this function, a function dependant on the portmacro.h header
 * 			function calls to request a context switch can be specified.
 * This can be used if sending to the queue from an ISR caused a task
 * to unblock and a context switch is required.
 */
void requestContextSwitch(CallContext callContext);

/**
 * If task preemption in FreeRTOS is disabled, a context switch
 * can be requested manually by calling this function.
 */
void vRequestContextSwitchFromTask(void);

/**
 * @return The current task handle
 */
TaskHandle_t getCurrentTaskHandle();

/**
 * Get returns the minimum amount of remaining stack space in words
 * that was a available to the task since the task started executing.
 * Please note that the actual value in bytes depends
 * on the stack depth type.
 * E.g. on a 32 bit machine, a value of 200 means 800 bytes.
 * @return Smallest value of stack remaining since the task was started in
 * 		   words.
 */
size_t getTaskStackHighWatermark(TaskHandle_t task = nullptr);

};  // namespace TaskManagement

#endif /* FRAMEWORK_OSAL_FREERTOS_TASKMANAGEMENT_H_ */
