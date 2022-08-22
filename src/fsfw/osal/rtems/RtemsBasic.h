#ifndef FSFW_OSAL_RTEMS_RTEMSBASIC_H_
#define FSFW_OSAL_RTEMS_RTEMSBASIC_H_

#include <rtems.h>
#include <rtems/error.h>
#include <rtems/libio.h>
#include <rtems/stackchk.h>

#include <cstddef>

#include "fsfw/returnvalues/returnvalue.h"

class RtemsBasic {
 public:
  static rtems_interval convertMsToTicks(uint32_t msIn) {
    rtems_interval ticks_per_second = rtems_clock_get_ticks_per_second();
    return (ticks_per_second * msIn) / 1000;
  }

  static rtems_interval convertTicksToMs(rtems_interval ticksIn) {
    rtems_interval ticks_per_second = rtems_clock_get_ticks_per_second();
    return (ticksIn * 1000) / ticks_per_second;
  }
};

#endif /* FSFW_OSAL_RTEMS_RTEMSBASIC_H_ */
