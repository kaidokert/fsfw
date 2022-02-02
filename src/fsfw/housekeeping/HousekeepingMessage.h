#ifndef FSFW_HOUSEKEEPING_HOUSEKEEPINGMESSAGE_H_
#define FSFW_HOUSEKEEPING_HOUSEKEEPINGMESSAGE_H_

#include "fsfw/datapoollocal/localPoolDefinitions.h"
#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/ipc/FwMessageTypes.h"
#include "fsfw/objectmanager/frameworkObjects.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

/**
 * @brief	Special command message type for housekeeping messages
 * @details
 * This message is slightly larger than regular command messages to accomodate
 * the uint64_t structure ID (SID).
 */
class HousekeepingMessage {
 public:
  static constexpr size_t HK_MESSAGE_SIZE =
      CommandMessageIF::HEADER_SIZE + sizeof(sid_t) + sizeof(uint32_t);

  /**
   * Concrete instance is not used, instead this class operates on
   * command message instances.
   */
  HousekeepingMessage() = delete;
  virtual ~HousekeepingMessage();

  static constexpr uint8_t MESSAGE_ID = messagetypes::HOUSEKEEPING;

  static constexpr Command_t ENABLE_PERIODIC_HK_REPORT_GENERATION = MAKE_COMMAND_ID(5);
  static constexpr Command_t DISABLE_PERIODIC_HK_REPORT_GENERATION = MAKE_COMMAND_ID(6);

  static constexpr Command_t ENABLE_PERIODIC_DIAGNOSTICS_GENERATION = MAKE_COMMAND_ID(7);
  static constexpr Command_t DISABLE_PERIODIC_DIAGNOSTICS_GENERATION = MAKE_COMMAND_ID(8);

  static constexpr Command_t REPORT_HK_REPORT_STRUCTURES = MAKE_COMMAND_ID(9);
  static constexpr Command_t REPORT_DIAGNOSTICS_REPORT_STRUCTURES = MAKE_COMMAND_ID(11);

  static constexpr Command_t HK_DEFINITIONS_REPORT = MAKE_COMMAND_ID(10);
  static constexpr Command_t DIAGNOSTICS_DEFINITION_REPORT = MAKE_COMMAND_ID(12);

  static constexpr Command_t HK_REPORT = MAKE_COMMAND_ID(25);
  static constexpr Command_t DIAGNOSTICS_REPORT = MAKE_COMMAND_ID(26);

  static constexpr Command_t GENERATE_ONE_PARAMETER_REPORT = MAKE_COMMAND_ID(27);
  static constexpr Command_t GENERATE_ONE_DIAGNOSTICS_REPORT = MAKE_COMMAND_ID(28);

  static constexpr Command_t MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL = MAKE_COMMAND_ID(31);
  static constexpr Command_t MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL = MAKE_COMMAND_ID(32);

  static constexpr Command_t HK_REQUEST_SUCCESS = MAKE_COMMAND_ID(128);
  static constexpr Command_t HK_REQUEST_FAILURE = MAKE_COMMAND_ID(129);

  static constexpr Command_t UPDATE_NOTIFICATION_SET = MAKE_COMMAND_ID(130);
  static constexpr Command_t UPDATE_NOTIFICATION_VARIABLE = MAKE_COMMAND_ID(131);

  static constexpr Command_t UPDATE_SNAPSHOT_SET = MAKE_COMMAND_ID(132);
  static constexpr Command_t UPDATE_SNAPSHOT_VARIABLE = MAKE_COMMAND_ID(133);

  // static constexpr Command_t UPDATE_HK_REPORT = MAKE_COMMAND_ID(134);

  static sid_t getSid(const CommandMessage* message);
  static gp_id_t getGpid(const CommandMessage* message);

  /* Housekeeping Interface Messages */

  static void setToggleReportingCommand(CommandMessage* command, sid_t sid, bool enableReporting,
                                        bool isDiagnostics);
  static void setStructureReportingCommand(CommandMessage* command, sid_t sid, bool isDiagnostics);
  static void setOneShotReportCommand(CommandMessage* command, sid_t sid, bool isDiagnostics);
  static void setCollectionIntervalModificationCommand(CommandMessage* command, sid_t sid,
                                                       float collectionInterval,
                                                       bool isDiagnostics);

  static void setHkReportReply(CommandMessage* reply, sid_t sid, store_address_t storeId);
  static void setHkDiagnosticsReply(CommandMessage* reply, sid_t sid, store_address_t storeId);

  static void setHkRequestSuccessReply(CommandMessage* reply, sid_t sid);
  static void setHkRequestFailureReply(CommandMessage* reply, sid_t sid, ReturnValue_t error);

  static void setHkStuctureReportReply(CommandMessage* reply, sid_t sid, store_address_t storeId);
  static void setDiagnosticsStuctureReportReply(CommandMessage* reply, sid_t sid,
                                                store_address_t storeId);

  static sid_t getHkRequestFailureReply(const CommandMessage* reply, ReturnValue_t* error);

  /**
   * @brief	Generic getter function for housekeeping data replies
   * @details
   * Command ID can be used beforehand to distinguish between diagnostics and
   * regular HK packets. This getter function should be used for the
   * command IDs 10, 12, 25 and 26.
   */
  static sid_t getHkDataReply(const CommandMessage* message, store_address_t* storeIdToSet);
  static sid_t getCollectionIntervalModificationCommand(const CommandMessage* command,
                                                        float* newCollectionInterval);

  /* Update Notification Messages */

  static void setUpdateNotificationSetCommand(CommandMessage* command, sid_t sid);
  static void setUpdateNotificationVariableCommand(CommandMessage* command, gp_id_t globalPoolId);

  static void setUpdateSnapshotSetCommand(CommandMessage* command, sid_t sid,
                                          store_address_t storeId);
  static void setUpdateSnapshotVariableCommand(CommandMessage* command, gp_id_t globalPoolId,
                                               store_address_t storeId);

  static sid_t getUpdateNotificationSetCommand(const CommandMessage* command);
  static gp_id_t getUpdateNotificationVariableCommand(const CommandMessage* command);

  static sid_t getUpdateSnapshotSetCommand(const CommandMessage* command, store_address_t* storeId);
  static gp_id_t getUpdateSnapshotVariableCommand(const CommandMessage* command,
                                                  store_address_t* storeId);

  /** Utility */
  static void clear(CommandMessage* message);

 private:
  static void setSid(CommandMessage* message, sid_t sid);
  static void setGpid(CommandMessage* message, gp_id_t globalPoolId);
};

#endif /* FSFW_HOUSEKEEPING_HOUSEKEEPINGMESSAGE_H_ */
