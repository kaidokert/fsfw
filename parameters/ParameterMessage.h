#ifndef FSFW_PARAMETERS_PARAMETERMESSAGE_H_
#define FSFW_PARAMETERS_PARAMETERMESSAGE_H_

#include "HasParametersIF.h"
#include "../ipc/CommandMessage.h"
#include "../storagemanager/StorageManagerIF.h"

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

#endif /* FSFW_PARAMETERS_PARAMETERMESSAGE_H_ */
