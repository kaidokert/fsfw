#ifndef FRAMEWORK_HK_HOUSEKEEPINGMESSAGE_H_
#define FRAMEWORK_HK_HOUSEKEEPINGMESSAGE_H_

#include <framework/ipc/CommandMessage.h>
#include <framework/ipc/FwMessageTypes.h>
#include <framework/objectmanager/SystemObjectIF.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <limits>

union sid_t {
	static constexpr uint64_t INVALID_ADDRESS =
			std::numeric_limits<uint64_t>::max();
	sid_t(): raw(INVALID_ADDRESS) {}

	struct {
		object_id_t objectId ;
		/**
		 * A generic 32 bit ID to identify unique HK packets for a single
		 * object. For example, the DeviceCommandId_t is used for
		 * DeviceHandlers
		 */
		uint32_t ownerSetId;
	};
	/**
	 * Alternative access to the raw value. This is also the size of the type.
	 */
	uint64_t raw;
};


/**
 * @brief	Special command message type for housekeeping messages
 * @details
 * This message is slightly larger than regular command messages to accomodate
 * the uint64_t structure ID (SID).
 */
class HousekeepingMessage {
public:

	static constexpr size_t HK_MESSAGE_SIZE = CommandMessageIF::HEADER_SIZE +
	        sizeof(sid_t) + sizeof(uint32_t);

	/**
	 * The HK message is initialized with a pointer to a message which holds
	 * the message data, see CommandMessageIF and getInternalMessage().
	 * @param message
	 */
	HousekeepingMessage() = delete;
	virtual ~HousekeepingMessage();

	static constexpr uint8_t MESSAGE_ID = messagetypes::HOUSEKEEPING;

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

	static sid_t getSid(const CommandMessage* message);

	static void setHkReportMessage(CommandMessage* message, sid_t sid,
			store_address_t storeId);
	static void setHkDiagnosticsMessage(CommandMessage* message, sid_t sid,
			store_address_t storeId);

	//! Get the respective SID and store ID. Command ID can be used beforehand
	//! to distinguish between diagnostics and regular HK packets
	static sid_t getHkReportMessage(const CommandMessage* message,
			store_address_t * storeIdToSet);

private:
	static void setSid(CommandMessage* message, sid_t sid);
	static void setParameter(CommandMessage* message, uint32_t parameter);
	static uint32_t getParameter(const CommandMessage* message);
};


#endif /* FRAMEWORK_HK_HOUSEKEEPINGMESSAGE_H_ */
