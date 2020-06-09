#ifndef PARAMETERMESSAGE_H_
#define PARAMETERMESSAGE_H_

#include <framework/ipc/CommandMessage.h>
#include <framework/parameters/HasParametersIF.h>
#include <framework/storagemanager/StorageManagerIF.h>

class ParameterMessage {
private:
	ParameterMessage();
public:
	static const uint8_t MESSAGE_ID = messagetypes::PARAMETER;
	static const Command_t CMD_PARAMETER_LOAD = MAKE_COMMAND_ID( 0x01 );
	static const Command_t CMD_PARAMETER_DUMP = MAKE_COMMAND_ID( 0x02 );
	static const Command_t REPLY_PARAMETER_DUMP = MAKE_COMMAND_ID( 0x03 );

	static ParameterId_t getParameterId(const CommandMessage* message);
	static store_address_t getStoreId(const CommandMessage* message);
	static void setParameterDumpCommand(CommandMessage* message,
			ParameterId_t id);
	static void setParameterDumpReply(CommandMessage* message,
			ParameterId_t id,  store_address_t storageID);
	static void setParameterLoadCommand(CommandMessage* message,
			ParameterId_t id, store_address_t storageID);
	static void clear(CommandMessage* message);

};

#endif /* PARAMETERMESSAGE_H_ */
