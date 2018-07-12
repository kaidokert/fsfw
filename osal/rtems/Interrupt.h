#ifndef OS_RTEMS_INTERRUPT_H_
#define OS_RTEMS_INTERRUPT_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <cstring>
#include <rtems.h>

typedef rtems_isr_entry IsrHandler_t;
typedef rtems_isr IsrReturn_t;
typedef rtems_vector_number InterruptNumber_t;

class Interrupt {
public:
	virtual ~Interrupt(){};

	/**
	 * Establishes a new interrupt service routine.
	 * @param handler	The service routine to establish
	 * @param interrupt	The interrupt (NOT trap type) the routine shall react to.
	 * @return	RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t setInterruptServiceRoutine(IsrHandler_t handler,
			InterruptNumber_t interrupt, IsrHandler_t *oldHandler = NULL);
	static ReturnValue_t enableInterrupt(InterruptNumber_t interruptNumber);
	static ReturnValue_t disableInterrupt(InterruptNumber_t interruptNumber);
	/**
	 * Enables the interrupt given.
	 * The function tests, if the InterruptMask register was written successfully.
	 * @param interrupt The interrupt to enable.
	 * @return RETURN_OK if the interrupt was set successfully. RETURN_FAILED else.
	 */
	static ReturnValue_t enableGpioInterrupt(InterruptNumber_t interrupt);
	/**
	 * Disables the interrupt given.
	 * @param interrupt The interrupt to disable.
	 * @return RETURN_OK if the interrupt was set successfully. RETURN_FAILED else.
	 */
	static ReturnValue_t disableGpioInterrupt(InterruptNumber_t interrupt);


	/**
	 * Checks if the current executing context is an ISR.
	 * @return true if handling an interrupt, false else.
	 */
	static bool isInterruptInProgress();

};


#endif /* OS_RTEMS_INTERRUPT_H_ */
