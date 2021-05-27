#ifndef MISSION_MEMORY_GENERICFILESYSTEMMESSAGE_H_
#define MISSION_MEMORY_GENERICFILESYSTEMMESSAGE_H_

#include <cstdint>

#include <fsfw/ipc/CommandMessageIF.h>
#include <fsfw/ipc/FwMessageTypes.h>
#include <fsfw/ipc/CommandMessage.h>
#include <fsfw/storagemanager/storeAddress.h>

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

    static void setLockFileCommand(CommandMessage* message, store_address_t storeId);
    static void setUnlockFileCommand(CommandMessage* message, store_address_t storeId);

    static void setCreateFileCommand(CommandMessage* message,
            store_address_t storeId);
    static void setDeleteFileCommand(CommandMessage* message,
            store_address_t storeId);

    static void setReportFileAttributesCommand(CommandMessage* message,
            store_address_t storeId);
    static void setReportFileAttributesReply(CommandMessage* message,
            store_address_t storeId);

    static void setCreateDirectoryCommand(CommandMessage* message,
            store_address_t storeId);
    static void setDeleteDirectoryCommand(CommandMessage* message,
            store_address_t storeId);

    static void setSuccessReply(CommandMessage* message);
    static void setFailureReply(CommandMessage* message,
            ReturnValue_t errorCode, uint32_t errorParam = 0);
};



#endif /* MISSION_MEMORY_GENERICFILESYSTEMMESSAGE_H_ */
