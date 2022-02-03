#ifndef MISSION_MEMORY_GENERICFILESYSTEMMESSAGE_H_
#define MISSION_MEMORY_GENERICFILESYSTEMMESSAGE_H_

#include <fsfw/ipc/CommandMessage.h>
#include <fsfw/ipc/CommandMessageIF.h>
#include <fsfw/ipc/FwMessageTypes.h>
#include <fsfw/storagemanager/storeAddress.h>

#include <cstdint>

/**
 * @brief   These messages are sent to an object implementing HasFilesystemIF.
 * @details
 * Enables a message-based file management. The user can add custo commands be implementing
 * this generic class.
 * @author  Jakob Meier, R. Mueller
 */
class GenericFileSystemMessage {
 public:
  /* Instantiation forbidden */
  GenericFileSystemMessage() = delete;

  static const uint8_t MESSAGE_ID = messagetypes::FILE_SYSTEM_MESSAGE;
  /* PUS standard  (ECSS-E-ST-70-41C15 2016 p.654) */
  static const Command_t CMD_CREATE_FILE = MAKE_COMMAND_ID(1);
  static const Command_t CMD_DELETE_FILE = MAKE_COMMAND_ID(2);
  /** Report file attributes */
  static const Command_t CMD_REPORT_FILE_ATTRIBUTES = MAKE_COMMAND_ID(3);
  static const Command_t REPLY_REPORT_FILE_ATTRIBUTES = MAKE_COMMAND_ID(4);
  /** Command to lock a file, setting it read-only */
  static const Command_t CMD_LOCK_FILE = MAKE_COMMAND_ID(5);
  /** Command to unlock a file, enabling further operations on it */
  static const Command_t CMD_UNLOCK_FILE = MAKE_COMMAND_ID(6);
  /**
   * Find file in repository, using a search pattern.
   * Please note that * is the wildcard character.
   * For example, when looking for all files which start with have the
   * structure tm<something>.bin, tm*.bin can be used.
   */
  static const Command_t CMD_FIND_FILE = MAKE_COMMAND_ID(7);
  static const Command_t CMD_CREATE_DIRECTORY = MAKE_COMMAND_ID(9);
  static const Command_t CMD_DELETE_DIRECTORY = MAKE_COMMAND_ID(10);
  static const Command_t CMD_RENAME_DIRECTORY = MAKE_COMMAND_ID(11);

  /** Dump contents of a repository */
  static const Command_t CMD_DUMP_REPOSITORY = MAKE_COMMAND_ID(12);
  /** Repository dump reply */
  static const Command_t REPLY_DUMY_REPOSITORY = MAKE_COMMAND_ID(13);
  static constexpr Command_t CMD_COPY_FILE = MAKE_COMMAND_ID(14);
  static constexpr Command_t CMD_MOVE_FILE = MAKE_COMMAND_ID(15);

  static const Command_t COMPLETION_SUCCESS = MAKE_COMMAND_ID(128);
  static const Command_t COMPLETION_FAILED = MAKE_COMMAND_ID(129);

  // These command IDs will remain until CFDP has been introduced and consolidated.
  /** Append operation commands */
  static const Command_t CMD_APPEND_TO_FILE = MAKE_COMMAND_ID(130);
  static const Command_t CMD_FINISH_APPEND_TO_FILE = MAKE_COMMAND_ID(131);
  static const Command_t REPLY_FINISH_APPEND = MAKE_COMMAND_ID(132);

  static const Command_t CMD_READ_FROM_FILE = MAKE_COMMAND_ID(140);
  static const Command_t REPLY_READ_FROM_FILE = MAKE_COMMAND_ID(141);
  static const Command_t CMD_STOP_READ = MAKE_COMMAND_ID(142);
  static const Command_t REPLY_READ_FINISHED_STOP = MAKE_COMMAND_ID(143);

  static void setLockFileCommand(CommandMessage* message, store_address_t storeId);
  static void setUnlockFileCommand(CommandMessage* message, store_address_t storeId);

  static void setCreateFileCommand(CommandMessage* message, store_address_t storeId);
  static void setDeleteFileCommand(CommandMessage* message, store_address_t storeId);

  static void setReportFileAttributesCommand(CommandMessage* message, store_address_t storeId);
  static void setReportFileAttributesReply(CommandMessage* message, store_address_t storeId);

  static void setCreateDirectoryCommand(CommandMessage* message, store_address_t storeId);
  static void setDeleteDirectoryCommand(CommandMessage* message, store_address_t storeId,
                                        bool deleteRecursively);
  static store_address_t getDeleteDirectoryCommand(const CommandMessage* message,
                                                   bool& deleteRecursively);

  static void setSuccessReply(CommandMessage* message);
  static void setFailureReply(CommandMessage* message, ReturnValue_t errorCode,
                              uint32_t errorParam = 0);
  static void setCopyCommand(CommandMessage* message, store_address_t storeId);

  static void setWriteCommand(CommandMessage* message, store_address_t storeId);
  static void setFinishStopWriteCommand(CommandMessage* message, store_address_t storeId);
  static void setFinishStopWriteReply(CommandMessage* message, store_address_t storeId);
  static void setFinishAppendReply(CommandMessage* message, store_address_t storeId);

  static void setReadCommand(CommandMessage* message, store_address_t storeId);
  static void setReadFinishedReply(CommandMessage* message, store_address_t storeId);
  static void setReadReply(CommandMessage* message, bool readFinished, store_address_t storeId);
  static bool getReadReply(const CommandMessage* message, store_address_t* storeId);

  static store_address_t getStoreId(const CommandMessage* message);
  static ReturnValue_t getFailureReply(const CommandMessage* message,
                                       uint32_t* errorParam = nullptr);

  static ReturnValue_t clear(CommandMessage* message);
};

#endif /* MISSION_MEMORY_GENERICFILESYSTEMMESSAGE_H_ */
