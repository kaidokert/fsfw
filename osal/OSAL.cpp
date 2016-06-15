/**
 * @file	OSAL.cpp
 * @brief	This file defines the OSAL class.
 * @date	19.12.2012
 * @author	baetz
 */

#include <framework/osal/OSAL.h>

#ifndef API
#error Please specify Operating System API. Supported: API=RTEMS_API
#elif API == RTEMS_API

#include <bsp_flp/bsp_flp.h>
//#include <bsp_flp/rmap/RMAPCookie.h>

ReturnValue_t OSAL::convertReturnCode(uint8_t inValue) {
	if (inValue == RTEMS_SUCCESSFUL) {
		return OSAL::RETURN_OK;
	} else {
		return MAKE_RETURN_CODE(inValue);
	}
}

Name_t OSAL::buildName(uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4) {
	return rtems_build_name(c1, c2, c3, c4);
}

Interval_t OSAL::getTicksPerSecond() {
	Interval_t ticks_per_second;
	(void) rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, &ticks_per_second);
	return ticks_per_second;
}

ReturnValue_t OSAL::setClock(TimeOfDay_t* time) {
	ReturnValue_t status = rtems_clock_set(time);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::setClock(timeval* time) {
	timespec newTime;
	newTime.tv_sec = time->tv_sec;
	newTime.tv_nsec = time->tv_usec * TOD_NANOSECONDS_PER_MICROSECOND;
	//TODO: Not sure if we need to protect this call somehow (by thread lock or something).
	_TOD_Set(&newTime);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t OSAL::getClock_timeval(timeval* time) {
	ReturnValue_t status = rtems_clock_get_tod_timeval(time);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::getClock_usecs(uint64_t* time) {
	timeval temp_time;
	uint8_t returnValue = rtems_clock_get_tod_timeval(&temp_time);
	*time = ((uint64_t) temp_time.tv_sec * 1000000) + temp_time.tv_usec;
	return OSAL::convertReturnCode(returnValue);
}

ReturnValue_t OSAL::getDateAndTime(TimeOfDay_t* time) {
	ReturnValue_t status = rtems_clock_get_tod(time);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::sleepFor(Interval_t ticks) {
	ReturnValue_t status = rtems_task_wake_after(ticks);
	status = convertReturnCode(status);
	return status;
}
ReturnValue_t OSAL::createTask(Name_t name, TaskPriority_t initial_priority,
		size_t stack_size, OpusMode_t initial_modes, Attribute_t attribute_set,
		TaskId_t* id) {
	if (initial_priority >= 0 && initial_priority <= 99) {
		ReturnValue_t status = rtems_task_create(name,
				(0xFF - 2 * initial_priority), stack_size, initial_modes,
				attribute_set, id);
		status = convertReturnCode(status);
		return status;
	} else {
		return OSAL::UNSATISFIED;
	}
}

ReturnValue_t OSAL::findTask(Name_t name, TaskId_t* id) {
	ReturnValue_t status = rtems_task_ident(name, RTEMS_SEARCH_ALL_NODES, id);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::startTask(TaskId_t* id, TaskEntry_t entry_point,
		TaskArgument_t argument) {
	return rtems_task_start(*id, entry_point, argument);
}

ReturnValue_t OSAL::deleteTask(TaskId_t* id) {
	if (id == RTEMS_SELF) {
		return rtems_task_delete(RTEMS_SELF);
	} else {
		return rtems_task_delete(*id);
	}
}

ReturnValue_t OSAL::setAndStartPeriod(Interval_t period, PeriodId_t *periodId,
		Name_t name) {
	uint8_t status;
	status = rtems_rate_monotonic_create(name, periodId);
	if (status == RTEMS_SUCCESSFUL) {
		//This is necessary to avoid a call with period = 0, which does not start the period.
		status = rtems_rate_monotonic_period(*periodId, period + 1);
	}
	return OSAL::convertReturnCode(status);
}

ReturnValue_t OSAL::checkAndRestartPeriod(PeriodId_t periodId,
		Interval_t period) {
	ReturnValue_t status = rtems_rate_monotonic_period(periodId, period);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::deletePeriod(TaskId_t* id) {
	rtems_name period_name;
	rtems_id period_id;
	rtems_object_get_classic_name(*id, &period_name);
	period_name = (period_name & 0xFFFFFF00) + 0x50;
	rtems_rate_monotonic_ident(period_name, &period_id);
	ReturnValue_t status = rtems_rate_monotonic_delete(period_id);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::reportPeriodStatistics() {
	rtems_rate_monotonic_report_statistics();
	return OSAL::SUCCESSFUL;
}

ReturnValue_t OSAL::createMessageQueue(Name_t name, uint32_t count,
		size_t max_message_size, Attribute_t attribute_set,
		MessageQueueId_t* id) {
	ReturnValue_t status = rtems_message_queue_create(name, count,
			max_message_size, attribute_set, id);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::findMessageQueue(Name_t name, MessageQueueId_t* id) {
	ReturnValue_t status = rtems_message_queue_ident(name,
	RTEMS_SEARCH_ALL_NODES, id);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::sendMessage(MessageQueueId_t id, const void* buffer,
		size_t size) {
	ReturnValue_t status = rtems_message_queue_send(id, buffer, size);
	return OSAL::convertReturnCode(status);
}

ReturnValue_t OSAL::receiveMessage(MessageQueueId_t id, void* buffer,
		size_t bufSize, size_t* recSize, Option_t option_set,
		Interval_t timeout) {
	ReturnValue_t status = rtems_message_queue_receive(id, buffer, recSize,
			option_set, timeout);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::flushMessageQueue(MessageQueueId_t id, uint32_t* count) {
	ReturnValue_t status = rtems_message_queue_flush(id, count);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::deleteMessageQueue(MessageQueueId_t* id) {
	ReturnValue_t status = rtems_message_queue_delete(*id);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::createMutex(Name_t name, MutexId_t* id) {
	ReturnValue_t status = rtems_semaphore_create(name, 1,
	RTEMS_SIMPLE_BINARY_SEMAPHORE, 0, id);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::deleteMutex(MutexId_t* id) {
	ReturnValue_t status = rtems_semaphore_delete(*id);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::lockMutex(MutexId_t* id, Interval_t timeout) {
	ReturnValue_t status = rtems_semaphore_obtain(*id, WAIT, timeout);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::getUptime(timeval* uptime) {
	timespec time;
	ReturnValue_t status = rtems_clock_get_uptime(&time);
	uptime->tv_sec = time.tv_sec;
	time.tv_nsec = time.tv_nsec / 1000;
	uptime->tv_usec = time.tv_nsec;
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::getUptime(uint32_t* uptimeMs) {
	*uptimeMs = rtems_clock_get_ticks_since_boot();
	return RTEMS_SUCCESSFUL;
}

ReturnValue_t OSAL::unlockMutex(MutexId_t* id) {
	ReturnValue_t status = rtems_semaphore_release(*id);
	status = convertReturnCode(status);
	return status;
}

ReturnValue_t OSAL::setInterruptServiceRoutine(IsrHandler_t handler,
		InterruptNumber_t interrupt, IsrHandler_t *oldHandler) {
	IsrHandler_t oldHandler_local;
	if (oldHandler == NULL) {
		oldHandler = &oldHandler_local;
	}
	//+ 0x10 comes because of trap type assignment to IRQs in UT699 processor
	ReturnValue_t status = rtems_interrupt_catch(handler, interrupt + 0x10,
			oldHandler);
	status = convertReturnCode(status);
	return status;
}

//TODO: Make default values (edge, polarity) settable?
ReturnValue_t OSAL::enableInterrupt(InterruptNumber_t interrupt) {
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
		return RETURN_OK;
	} else {
		return RETURN_FAILED;
	}
}

ReturnValue_t OSAL::disableInterrupt(InterruptNumber_t interrupt) {
	volatile uint32_t* irqMask = hw_irq_mask;
	uint32_t expectedValue = *irqMask & ~(1 << interrupt);
	*irqMask = expectedValue;
	uint32_t tempValue = *irqMask;
	if (tempValue == expectedValue) {
		//Disable gpio IRQ
		volatile hw_gpio_port_t* ioPorts = hw_gpio_port;
		ioPorts->interrupt_mask &= ~(1 << interrupt);
		return RETURN_OK;
	} else {
		return RETURN_FAILED;
	}
}

bool OSAL::isInterruptInProgress() {
	return rtems_interrupt_is_in_progress();
}

ReturnValue_t OSAL::convertTimeOfDayToTimeval(const TimeOfDay_t* from,
		timeval* to) {
	//Fails in 2038..
	to->tv_sec = _TOD_To_seconds(from);
	to->tv_usec = 0;
	return RETURN_OK;
}

#endif /* API */
