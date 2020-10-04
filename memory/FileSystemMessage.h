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
	static const Command_t CREATE_FILE = MAKE_COMMAND_ID(1);
	static const Command_t DELETE_FILE = MAKE_COMMAND_ID(2);
	static const Command_t CREATE_DIRECTORY = MAKE_COMMAND_ID(9);
	static const Command_t DELETE_DIRECTORY = MAKE_COMMAND_ID(10);
	static const Command_t WRITE = MAKE_COMMAND_ID(128);
	static const Command_t READ = MAKE_COMMAND_ID(129);
	static const Command_t READ_REPLY = MAKE_COMMAND_ID(130);
	static const Command_t COMPLETION_SUCCESS = MAKE_COMMAND_ID(5);
	static const Command_t COMPLETION_FAILED = MAKE_COMMAND_ID(6);

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
