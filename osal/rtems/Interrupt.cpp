#include "Interrupt.h"
extern "C" {
#include <bsp_flp/hw_timer/hw_timer.h>
#include <bsp_flp/hw_uart/hw_uart.h>
}
#include "RtemsBasic.h"


ReturnValue_t Interrupt::enableInterrupt(InterruptNumber_t interruptNumber) {
	volatile uint32_t* irqMask = hw_irq_mask;
	uint32_t expectedValue = *irqMask | (1 << interruptNumber);
	*irqMask = expectedValue;
	uint32_t tempValue = *irqMask;
	if (tempValue == expectedValue) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t Interrupt::setInterruptServiceRoutine(IsrHandler_t handler,
		InterruptNumber_t interrupt, IsrHandler_t* oldHandler) {
	IsrHandler_t oldHandler_local;
	if (oldHandler == NULL) {
		oldHandler = &oldHandler_local;
	}
	//+ 0x10 comes because of trap type assignment to IRQs in UT699 processor
	rtems_status_code status = rtems_interrupt_catch(handler, interrupt + 0x10,
			oldHandler);
	switch(status){
	case RTEMS_SUCCESSFUL:
		//ISR established successfully
		return HasReturnvaluesIF::RETURN_OK;
	case RTEMS_INVALID_NUMBER:
		//illegal vector number
		return HasReturnvaluesIF::RETURN_FAILED;
	case RTEMS_INVALID_ADDRESS:
		//illegal ISR entry point or invalid old_isr_handler
		return HasReturnvaluesIF::RETURN_FAILED;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}

}

ReturnValue_t Interrupt::disableInterrupt(InterruptNumber_t interruptNumber) {
	//TODO Not implemented
	return HasReturnvaluesIF::RETURN_FAILED;
}

//SHOULDDO: Make default values (edge, polarity) settable?
ReturnValue_t Interrupt::enableGpioInterrupt(InterruptNumber_t interrupt) {
	volatile uint32_t* irqMask = hw_irq_mask;
	uint32_t expectedValue = *irqMask | (1 << interrupt);
	*irqMask = expectedValue;
	uint32_t tempValue = *irqMask;
	if (tempValue == expectedValue) {
		volatile hw_gpio_port_t* ioPorts = hw_gpio_port;
		ioPorts->direction &= ~(1 << interrupt); //Direction In
		ioPorts->interrupt_edge |= 1 << interrupt; //Edge triggered
		ioPorts->interrupt_polarity |= 1 << interrupt; //Trigger on rising edge
		ioPorts->interrupt_mask |= 1 << interrupt; //Enable
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t Interrupt::disableGpioInterrupt(InterruptNumber_t interrupt) {
	volatile uint32_t* irqMask = hw_irq_mask;
	uint32_t expectedValue = *irqMask & ~(1 << interrupt);
	*irqMask = expectedValue;
	uint32_t tempValue = *irqMask;
	if (tempValue == expectedValue) {
		//Disable gpio IRQ
		volatile hw_gpio_port_t* ioPorts = hw_gpio_port;
		ioPorts->interrupt_mask &= ~(1 << interrupt);
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

bool Interrupt::isInterruptInProgress() {
	return rtems_interrupt_is_in_progress();
}
