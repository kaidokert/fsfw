/*
 * MemoryMessage.h
 *
 *  Created on: 17.07.2013
 *      Author: Bastian
 */

#ifndef MEMORYMESSAGE_H_
#define MEMORYMESSAGE_H_

#include <framework/ipc/CommandMessage.h>
#include <framework/storagemanager/StorageManagerIF.h>


class MemoryMessage {
private:
	MemoryMessage(); //A private ctor inhibits instantiation
public:
	static const uint8_t MESSAGE_ID = MEMORY_MESSAGE_ID;
	static const Command_t CMD_MEMORY_LOAD = MAKE_COMMAND_ID( 0x01 );
	static const Command_t CMD_MEMORY_DUMP = MAKE_COMMAND_ID( 0x02 );
	static const Command_t CMD_MEMORY_CHECK = MAKE_COMMAND_ID( 0x03 );
	static const Command_t REPLY_MEMORY_DUMP = MAKE_COMMAND_ID( 0x10 );
	static const Command_t REPLY_MEMORY_CHECK = MAKE_COMMAND_ID( 0x30 );
	static const Command_t REPLY_MEMORY_FAILED = MAKE_COMMAND_ID( 0xE0 );

	static uint32_t getAddress( const CommandMessage* message );
	static store_address_t getStoreID( const CommandMessage* message );
	static uint32_t getLength( const CommandMessage* message );
	static ReturnValue_t getErrorCode( const CommandMessage* message );
	static ReturnValue_t setMemoryDumpCommand( CommandMessage* message, uint32_t address, uint32_t length );
	static ReturnValue_t setMemoryDumpReply( CommandMessage* message, store_address_t storageID );
	static ReturnValue_t setMemoryLoadCommand( CommandMessage* message, uint32_t address, store_address_t storageID );
	static ReturnValue_t setMemoryCheckCommand( CommandMessage* message, uint32_t address, uint32_t length );
	static ReturnValue_t setMemoryCheckReply(  CommandMessage* message, uint16_t crc );
	static ReturnValue_t setMemoryReplyFailed(  CommandMessage* message, ReturnValue_t errorCode, Command_t initialCommand );
	static uint16_t getCrc( const CommandMessage* message );
	static Command_t getInitialCommand( const CommandMessage* message );
	static void clear(CommandMessage* message);
};

#endif /* MEMORYMESSAGE_H_ */
