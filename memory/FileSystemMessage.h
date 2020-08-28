/*
 * FileSystemMessage.h
 *
 *  Created on: 19.01.2020
 *      Author: Jakob Meier
 */

#ifndef FRAMEWORK_MEMORY_FILESYSTEMMESSAGE_H_
#define FRAMEWORK_MEMORY_FILESYSTEMMESSAGE_H_

#include "../ipc/CommandMessage.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../objectmanager/SystemObject.h"

class FileSystemMessage {
private:
	FileSystemMessage(); //A private ctor inhibits instantiation
public:
	static const uint8_t MESSAGE_ID = messagetypes::FILE_SYSTEM_MESSAGE;
	static const Command_t CREATE_FILE = MAKE_COMMAND_ID( 0x01 );
	static const Command_t DELETE_FILE = MAKE_COMMAND_ID( 0x02 );
	static const Command_t WRITE_TO_FILE = MAKE_COMMAND_ID( 0x80 );

	static ReturnValue_t setWriteToFileCommand(CommandMessage* message, MessageQueueId_t replyToQueue, store_address_t storageID );
	static store_address_t getStoreID( const CommandMessage* message );
	static MessageQueueId_t getReplyQueueId(const CommandMessage* message);

};

#endif /* FRAMEWORK_MEMORY_FILESYSTEMMESSAGE_H_ */
