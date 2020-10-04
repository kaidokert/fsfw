#ifndef FSFW_MEMORY_FILESYSTEMMESSAGE_H_
#define FSFW_MEMORY_FILESYSTEMMESSAGE_H_

#include "../ipc/CommandMessage.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../objectmanager/SystemObject.h"

/**
 * @author  Jakob Meier
 */
class FileSystemMessage {
public:
    // Instantiation forbidden
	FileSystemMessage() = delete;

	static const uint8_t MESSAGE_ID = messagetypes::FILE_SYSTEM_MESSAGE;
	/* PUS standard  (ECSS-E-ST-70-41C15 2016 p.654) */
	static const Command_t CREATE_FILE = MAKE_COMMAND_ID(1);
	static const Command_t DELETE_FILE = MAKE_COMMAND_ID(2);
	/** Report file attributes */
	static const Command_t REPORT_FILE_ATTRIBUTES = MAKE_COMMAND_ID(3);
	static const Command_t FILE_ATTRIBUTES_REPLY = MAKE_COMMAND_ID(4);
	/** Command to lock a file, setting it read-only */
	static const Command_t LOCK_FILE = MAKE_COMMAND_ID(5);
	/** Command to unlock a file, enabling further operations on it */
	static const Command_t UNLOCK_FILE = MAKE_COMMAND_ID(6);
	/**
	 * Find file in repository, using a search pattern.
	 * Please note that * is the wildcard character.
	 * For example, when looking for all files which start with have the
	 * structure tm<something>.bin, tm*.bin can be used.
	 */
	static const Command_t FIND_FILE = MAKE_COMMAND_ID(7);
	static const Command_t CREATE_DIRECTORY = MAKE_COMMAND_ID(9);
    static const Command_t DELETE_DIRECTORY = MAKE_COMMAND_ID(10);
	static const Command_t RENAME_DIRECTORY = MAKE_COMMAND_ID(11);

	/** Dump contents of a repository */
	static const Command_t DUMP_REPOSITORY = MAKE_COMMAND_ID(12);
	/** Repository dump reply */
	static const Command_t DUMY_REPOSITORY_REPLY = MAKE_COMMAND_ID(13);

    static const Command_t APPEND_TO_FILE = MAKE_COMMAND_ID(128);
    static const Command_t READ_FROM_FILE = MAKE_COMMAND_ID(129);
    static const Command_t READ_REPLY = MAKE_COMMAND_ID(130);

    /** Dump the structure of the whole SD card as an ASCII file */
    static const Command_t DUMP_FILE_STRUCTURE = MAKE_COMMAND_ID(131);


	static const Command_t COMPLETION_SUCCESS = MAKE_COMMAND_ID(150);
	static const Command_t COMPLETION_FAILED = MAKE_COMMAND_ID(151);

	static void setCreateFileCommand(CommandMessage* message,
	        store_address_t storeId);
	static void setDeleteFileCommand(CommandMessage* message,
	        store_address_t storeId);
	static void setCreateDirectoryCommand(CommandMessage* message,
	        store_address_t storeId);
	static void setDeleteDirectoryCommand(CommandMessage* message,
	        store_address_t storeId);
	static void setWriteCommand(CommandMessage* message,
	        store_address_t storeId);
	static void setReadCommand(CommandMessage* message,
	        store_address_t storeId);
	static void setReadReply(CommandMessage* message, store_address_t storeId);
    static void setSuccessReply(CommandMessage* message);
    static void setFailureReply(CommandMessage* message,
            ReturnValue_t errorCode);

	static store_address_t getStoreId(const CommandMessage* message);
	static ReturnValue_t getFailureReply(const CommandMessage* message);
};

#endif /* FSFW_MEMORY_FILESYSTEMMESSAGE_H_ */
