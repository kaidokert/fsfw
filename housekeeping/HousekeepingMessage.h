#ifndef FSFW_HOUSEKEEPING_HOUSEKEEPINGMESSAGE_H_
#define FSFW_HOUSEKEEPING_HOUSEKEEPINGMESSAGE_H_

#include "../ipc/CommandMessage.h"
#include "../ipc/FwMessageTypes.h"
#include "../objectmanager/frameworkObjects.h"
#include "../objectmanager/SystemObjectIF.h"
#include "../storagemanager/StorageManagerIF.h"

union sid_t {
	static constexpr uint64_t INVALID_SID = -1;
	static constexpr uint32_t INVALID_SET_ID = -1;
	static constexpr uint32_t INVALID_OBJECT_ID = objects::NO_OBJECT;
	sid_t(): raw(INVALID_SID) {}

	sid_t(object_id_t objectId, uint32_t setId):
			objectId(objectId),
			ownerSetId(setId) {}

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

	bool notSet() const {
	    return raw == INVALID_SID;
	}

	bool operator==(const sid_t& other) const {
		return raw == other.raw;
	}

	bool operator!=(const sid_t& other) const {
		return not (raw == other.raw);
	}
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
	 * Concrete instance is not used, instead this class operates on
	 * command message instances.
	 */
	HousekeepingMessage() = delete;
	virtual ~HousekeepingMessage();

	static constexpr uint8_t MESSAGE_ID = messagetypes::HOUSEKEEPING;

	static constexpr Command_t ENABLE_PERIODIC_HK_REPORT_GENERATION =
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

	static constexpr Command_t MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL =
			MAKE_COMMAND_ID(31);
	static constexpr Command_t MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL =
			MAKE_COMMAND_ID(32);

	static constexpr Command_t HK_REQUEST_SUCCESS =
			MAKE_COMMAND_ID(128);
	static constexpr Command_t HK_REQUEST_FAILURE =
			MAKE_COMMAND_ID(129);

	static constexpr Command_t UPDATE_NOTIFICATION = MAKE_COMMAND_ID(130);
	static constexpr Command_t UPDATE_SNAPSHOT = MAKE_COMMAND_ID(131);

	static constexpr Command_t UPDATE_HK_REPORT = MAKE_COMMAND_ID(132);

	static sid_t getSid(const CommandMessage* message);

	/** Setter functions */
	static void setToggleReportingCommand(CommandMessage* command, sid_t sid,
			bool enableReporting, bool isDiagnostics);
	static void setStructureReportingCommand(CommandMessage* command, sid_t sid,
			bool isDiagnostics);
	static void setOneShotReportCommand(CommandMessage* command, sid_t sid,
			bool isDiagnostics);
	static void setCollectionIntervalModificationCommand(
			CommandMessage* command, sid_t sid, float collectionInterval,
			bool isDiagnostics);

	static void setHkReportReply(CommandMessage* reply, sid_t sid,
			store_address_t storeId);
	static void setHkDiagnosticsReply(CommandMessage* reply, sid_t sid,
			store_address_t storeId);

	static void setHkRequestSuccessReply(CommandMessage* reply, sid_t sid);
	static void setHkRequestFailureReply(CommandMessage* reply, sid_t sid,
			ReturnValue_t error);

	static void setHkStuctureReportReply(CommandMessage* reply,
	        sid_t sid, store_address_t storeId);
	static void setDiagnosticsStuctureReportReply(CommandMessage* reply,
	            sid_t sid, store_address_t storeId);

	static sid_t getHkRequestFailureReply(const CommandMessage* reply,
			ReturnValue_t* error);

	/**
	 * @brief	Generic getter function for housekeeping data replies
	 * @details
	 * Command ID can be used beforehand to distinguish between diagnostics and
	 * regular HK packets. This getter function should be used for the
	 * command IDs 10, 12, 25 and 26.
	 */
	static sid_t getHkDataReply(const CommandMessage* message,
			store_address_t * storeIdToSet);
	static sid_t getCollectionIntervalModificationCommand(
			const CommandMessage* command, float* newCollectionInterval);

	static void clear(CommandMessage* message);
private:
	static void setSid(CommandMessage* message, sid_t sid);
};


#endif /* FSFW_HOUSEKEEPING_HOUSEKEEPINGMESSAGE_H_ */
