#ifndef MONITORINGMESSAGE_H_
#define MONITORINGMESSAGE_H_

#include <framework/ipc/CommandMessage.h>
#include <framework/storagemanager/StorageManagerIF.h>

class MonitoringMessage: public CommandMessage {
public:
	static const uint8_t MESSAGE_ID = LIMIT_MESSAGE_ID;
	static const Command_t ADD_MONITOR = MAKE_COMMAND_ID(1); //PID(uint32_t), Data(type, {LIMIT_ID(uint8_t), initialLimits(data, depends)})
	static const Command_t UPDATE_PARAMETER_MONITOR = MAKE_COMMAND_ID(2); //PID(uint32_t), Data{type, n_entries {LIMIT_ID(uint8_t), TYPE(uint8_t) newLimits(data, depends)
	static const Command_t UPDATE_OBJECT_MONITOR = MAKE_COMMAND_ID(3);
	static const Command_t CHANGE_REPORTING_STRATEGY = MAKE_COMMAND_ID(4); //PID(uint32_t), Data{type, n_entries {LIMIT_ID(uint8_t), TYPE(uint8_t) newLimits(data, depends)
	//Optional
//	static const Command_t REPORT_LIMIT_DEFINITIONS = MAKE_COMMAND_ID(3); //N_PIDS, PID(uint32_t)
//	static const Command_t LIMIT_DEFINITION_REPORT = MAKE_COMMAND_ID(3); //Eventually multiple reports per type or even per definition.
	//Object id could be useful, but we better manage that on service level (register potential reporters).
	static const Command_t LIMIT_VIOLATION_REPORT = MAKE_COMMAND_ID(10);
	virtual ~MonitoringMessage();
	static void setAddLimitCommand(CommandMessage* message, store_address_t storeId);
	static void setChangeReportingStrategy(CommandMessage* message, uint8_t strategy, store_address_t storeId);
	static void setLimitViolationReport(CommandMessage* message, store_address_t storeId);
	static void clear(CommandMessage* message);
	static store_address_t getStoreId(const CommandMessage* message);
	static uint8_t getReportingStategy(CommandMessage* message);
	static void setTypicalMessage(CommandMessage* message, Command_t type, store_address_t storeId);

};


#endif /* MONITORINGMESSAGE_H_ */
