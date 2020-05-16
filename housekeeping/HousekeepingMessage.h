#ifndef FRAMEWORK_HK_HOUSEKEEPINGMESSAGE_H_
#define FRAMEWORK_HK_HOUSEKEEPINGMESSAGE_H_
#include <framework/ipc/CommandMessage.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <limits>

/**
 * the sid consists of the target object ID and... something else I forgot.
 * Propably a special HK id to distinguish multiple hk pool packages
 * inside a handler or controller
 */
typedef uint32_t set_t;

union sid_t {
	static constexpr uint64_t INVALID_ADDRESS = std::numeric_limits<uint64_t>::max();
	sid_t(): raw(INVALID_ADDRESS) {}

	struct {
		object_id_t objectId ;
		set_t hkId;
	};
	/**
	 * Alternative access to the raw value.
	 */
	uint64_t raw;
};
class HousekeepingMessage {
public:
	/**
	 * No instances of a message shall be created, instead
	 * a CommandMessage instance is manipulated.
	 */
	HousekeepingMessage() = delete;
	HousekeepingMessage(const HousekeepingMessage&) = delete;
	HousekeepingMessage operator=(const HousekeepingMessage &) = delete;

	static constexpr uint8_t MESSAGE_ID = MESSAGE_TYPE::HOUSEKEEPING;
	static constexpr Command_t ADD_HK_REPORT_STRUCT =
			MAKE_COMMAND_ID(1);
	static constexpr Command_t ADD_DIAGNOSTICS_REPORT_STRUCT =
			MAKE_COMMAND_ID(2);

	static constexpr Command_t DELETE_HK_REPORT_STRUCT = MAKE_COMMAND_ID(3);
	static constexpr Command_t DELETE_DIAGNOSTICS_REPORT_STRUCT =
			MAKE_COMMAND_ID(4);

	static constexpr Command_t ENABLE_PERIODIC_HK_GENERATION =
			MAKE_COMMAND_ID(5);
	static constexpr Command_t DISABLE_PERIODIC_HK_REPORT_GENERATION =
			MAKE_COMMAND_ID(6);

	static constexpr Command_t ENABLE_PERIODIC_DIAGNOSTICS_GENERATION =
			MAKE_COMMAND_ID(7);
	static constexpr Command_t DISABLE_PERIODIC_DIAGNOSTICS_GENERATION =
			MAKE_COMMAND_ID(8);

	static constexpr Command_t REPORT_HK_REPORT_STRUCTURES = MAKE_COMMAND_ID(9);
	static constexpr Command_t REPORT_DIAGNOSTICS_REPORT_STRUCTURES =
			MAKE_COMMAND_ID(11);

	static constexpr Command_t HK_DEFINITIONS_REPORT = MAKE_COMMAND_ID(10);
	static constexpr Command_t DIAGNOSTICS_DEFINITION_REPORT = MAKE_COMMAND_ID(12);

	static constexpr Command_t HK_REPORT = MAKE_COMMAND_ID(25);
	static constexpr Command_t DIAGNOSTICS_REPORT = MAKE_COMMAND_ID(26);

	static constexpr Command_t GENERATE_ONE_PARAMETER_REPORT  =
			MAKE_COMMAND_ID(27);
	static constexpr Command_t GENERATE_ONE_DIAGNOSTICS_REPORT =
			MAKE_COMMAND_ID(28);

	static constexpr Command_t APPEND_PARAMETERS_TO_PARAMETER_REPORT_STRUCTURE =
			MAKE_COMMAND_ID(29);
	static constexpr Command_t APPEND_PARAMETERS_TO_DIAGNOSTICS_REPORT_STRUCTURE =
			MAKE_COMMAND_ID(30);

	static constexpr Command_t MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL =
			MAKE_COMMAND_ID(31);
	static constexpr Command_t MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL =
			MAKE_COMMAND_ID(32);

	static void setAddHkReportStructMessage(CommandMessage* message,
			set_t setId, store_address_t packet);
};


#endif /* FRAMEWORK_HK_HOUSEKEEPINGMESSAGE_H_ */
