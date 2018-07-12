#ifndef OS_RTEMS_RTEMSBASIC_H_
#define OS_RTEMS_RTEMSBASIC_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/osal/OperatingSystemIF.h>
extern "C" {
#include <bsp_flp/rtems_config.h>
}
#include <rtems/endian.h>
#include <rtems.h>
#include <rtems/libio.h>
#include <rtems/error.h>
#include <rtems/stackchk.h>
#include <stddef.h>


class RtemsBasic: public OperatingSystemIF {
public:
	/**
	 * A method to convert an OS-specific return code to the frameworks return value concept.
	 * @param inValue The return code coming from the OS.
	 * @return The converted return value.
	 */
	static ReturnValue_t convertReturnCode(rtems_status_code inValue);

	static rtems_interval convertMsToTicks(uint32_t msIn) {
		rtems_interval ticks_per_second;
		rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, &ticks_per_second);
		return (ticks_per_second * msIn) / 1000;
	}

	static rtems_interval convertTicksToMs(rtems_interval ticksIn) {
		rtems_interval ticks_per_second;
		rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, &ticks_per_second);
		return (ticksIn * 1000) / ticks_per_second;
	}
};

#endif /* OS_RTEMS_RTEMSBASIC_H_ */
