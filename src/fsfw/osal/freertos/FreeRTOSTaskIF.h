#ifndef FSFW_OSAL_FREERTOS_FREERTOSTASKIF_H_
#define FSFW_OSAL_FREERTOS_FREERTOSTASKIF_H_

#include "FreeRTOS.h"
#include "task.h"

class FreeRTOSTaskIF {
 public:
  virtual ~FreeRTOSTaskIF() = default;
  virtual TaskHandle_t getTaskHandle() = 0;

 protected:
  static bool checkMissedDeadline(const TickType_t xLastWakeTime, const TickType_t interval) {
    /* Check whether deadline was missed while also taking overflows
     * into account. Drawing this on paper with a timeline helps to understand
     * it. */
    TickType_t currentTickCount = xTaskGetTickCount();
    TickType_t timeToWake = xLastWakeTime + interval;
    // Time to wake has not overflown.
    if (timeToWake > xLastWakeTime) {
      /* If the current time has overflown exclusively or the current
       * tick count is simply larger than the time to wake, a deadline was
       * missed */
      if ((currentTickCount < xLastWakeTime) or (currentTickCount > timeToWake)) {
        return true;
      }
    }
    /* Time to wake has overflown. A deadline was missed if the current time
     * is larger than the time to wake */
    else if ((timeToWake < xLastWakeTime) and (currentTickCount > timeToWake)) {
      return true;
    }
    return false;
  }
};

#endif /* FSFW_OSAL_FREERTOS_FREERTOSTASKIF_H_ */
