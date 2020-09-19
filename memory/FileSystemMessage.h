#ifndef FSFW_MEMORY_FILESYSTEMMESSAGE_H_
#define FSFW_MEMORY_FILESYSTEMMESSAGE_H_

#include "../ipc/CommandMessage.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../objectmanager/SystemObject.h"

/**
 * @author  Jakob Meier
 */
class FileSystemMessage {
private:
	FileSystemMessage(); //A private ctor inhibits instantiation
public:
	static const uint8_t MESSAGE_ID = messagetypes::FILE_SYSTEM_MESSAGE;
	static const Command_t CREATE_FILE = MAKE_COMMAND_ID( 0x01 );
	static const Command_t DELETE_FILE = MAKE_COMMAND_ID( 0x02 );
	static const Command_t CREATE_DIRECTORY = MAKE_COMMAND_ID( 0x09 );
	static const Command_t DELETE_DIRECTORY = MAKE_COMMAND_ID( 0x0A );
	static const Command_t WRITE = MAKE_COMMAND_ID( 0x80 );
	static const Command_t READ = MAKE_COMMAND_ID( 0x81 );
	static const Command_t READ_REPLY = MAKE_COMMAND_ID( 0x82 );
	static const Command_t COMPLETION_SUCCESS = MAKE_COMMAND_ID(5);
	static const Command_t COMPLETION_FAILED = MAKE_COMMAND_ID(6);

	static ReturnValue_t setDeleteFileCommand(CommandMessage* message, store_address_t storageID);
	static ReturnValue_t setCreateDirectoryCommand(CommandMessage* message, store_address_t storageID);
	static ReturnValue_t setDeleteDirectoryCommand(CommandMessage* message, store_address_t storageID);
	static ReturnValue_t setWriteCommand(CommandMessage* message, store_address_t storageID);
	static ReturnValue_t setReadCommand(CommandMessage* message, store_address_t storageID);
	static ReturnValue_t setReadReply(CommandMessage* message, store_address_t storageID);
	static store_address_t getStoreId( const CommandMessage* message );
	static void setCompletionReply(CommandMessage* message, Command_t completionStatus);
};

#endif /* FSFW_MEMORY_FILESYSTEMMESSAGE_H_ */
