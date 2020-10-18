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

	/**
	 * Command to set a load parameter message. The CCSDS / ECSS type in
	 * form of a PTC and a PFC is expected. See ECSS-E-ST-70-41C15 p.428
	 * for all types or the Type class in globalfunctions.
	 * @param message
	 * @param id
	 * @param storeId
	 * @param ptc Type information according to CCSDS/ECSS standards
	 * @param pfc Type information according to CCSDS/ECSS standards
	 * @param rows Set number of rows in parameter set, minimum one.
	 * @param columns Set number of columns in parameter set, minimum one
	 */
	static void setParameterLoadCommand(CommandMessage* message,
	        ParameterId_t id, store_address_t storeId, uint8_t ptc,
	        uint8_t pfc, uint8_t rows, uint8_t columns);
	static void clear(CommandMessage* message);

};

#endif /* FSFW_PARAMETERS_PARAMETERMESSAGE_H_ */
