#include <framework/watchdog/Watchdog.h>

extern "C" {
#include <bsp_flp/hw_timer/hw_timer.h>
}

#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <ostream>

Watchdog::Watchdog(object_id_t objectId, uint32_t interval_us, uint32_t initial_interval_us) :
		SystemObject(objectId), commandQueue(3,
				WatchdogMessage::WATCHDOG_MESSAGE_SIZE) {
	hw_timer_set_reload_value(hw_timer_watchdog, interval_us);
	hw_timer_start_(hw_timer_watchdog, initial_interval_us);
	hw_timer_watchdog ->control_register |= 8;

	hw_gpio_port ->direction |= (HW_GPIO_DDR_OUT << HW_GPIO_LEON_WD_EN);
	hw_gpio_port ->output |= (HW_GPIO_DDR_OUT << HW_GPIO_LEON_WD_EN);
}

Watchdog::~Watchdog() {
}

ReturnValue_t Watchdog::performOperation() {
	WatchdogMessage message;
	ReturnValue_t result = commandQueue.receiveMessage(&message);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		hw_timer_reload(hw_timer_watchdog );
	} else {
		debug << "Watchdog::performOperation: Object 0x" << std::hex
				<< message.getSender() << std::dec << " requested ";
		switch (message.getCommand()) {
		case WatchdogMessage::ENABLE:
			debug << "watchdog enable" << std::endl;
			hw_timer_reload(hw_timer_watchdog );
			hw_timer_start(hw_timer_watchdog );
			break;
		case WatchdogMessage::DISABLE:
			debug << "watchdog disable" << std::endl;
			hw_timer_stop(hw_timer_watchdog );
			break;
		case WatchdogMessage::RESET_CPU:
			debug << "CPU reset" << std::endl;
			hw_timer_start_(hw_timer_watchdog, 10);
			break;
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t Watchdog::getCommandQueue() {
	return commandQueue.getId();
}

