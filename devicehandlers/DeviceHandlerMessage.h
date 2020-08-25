#ifndef DEVICEHANDLERMESSAGE_H_
#define DEVICEHANDLERMESSAGE_H_

#include "../action/ActionMessage.h"
#include "../ipc/CommandMessage.h"
#include "../objectmanager/SystemObjectIF.h"
#include "../storagemanager/StorageManagerIF.h"
//SHOULDDO: rework the static constructors to name the type of command they are building, maybe even hide setting the commandID.

/**
 * This is used to uniquely identify commands that are sent to a device
 *
 * The values are defined in the device-specific implementations
 */
typedef uint32_t DeviceCommandId_t;

/**
 * The DeviceHandlerMessage is used to send Commands to a DeviceHandlerIF
 */
class DeviceHandlerMessage {
private:
	DeviceHandlerMessage();
public:

	/**
	 * These are the commands that can be sent to a DeviceHandlerBase
	 */
	static const uint8_t MESSAGE_ID = messagetypes::DEVICE_HANDLER_COMMAND;
	static const Command_t CMD_RAW = MAKE_COMMAND_ID( 1 ); //!< Sends a raw command, setParameter is a ::store_id_t containing the raw packet to send
//	static const Command_t CMD_DIRECT = MAKE_COMMAND_ID( 2 ); //!< Sends a direct command, setParameter is a ::DeviceCommandId_t, setParameter2 is a ::store_id_t containing the data needed for the command
	static const Command_t CMD_SWITCH_IOBOARD = MAKE_COMMAND_ID( 3 ); //!< Requests a IO-Board switch, setParameter() is the IO-Board identifier
	static const Command_t CMD_WIRETAPPING = MAKE_COMMAND_ID( 4 ); //!< (De)Activates the monitoring of all raw traffic in DeviceHandlers, setParameter is 0 to deactivate, 1 to activate

	/*static const Command_t REPLY_SWITCHED_IOBOARD = MAKE_COMMAND_ID(1 );//!< Reply to a @c CMD_SWITCH_IOBOARD, indicates switch was successful, getParameter() contains the board switched to (0: nominal, 1: redundant)
	 static const Command_t REPLY_CANT_SWITCH_IOBOARD = MAKE_COMMAND_ID( 2);	//!< Reply to a @c CMD_SWITCH_IOBOARD, indicating the switch could not be performed, getParameter() contains the error message
	 static const Command_t REPLY_WIRETAPPING = MAKE_COMMAND_ID( 3);	//!< Reply to a @c CMD_WIRETAPPING, getParameter() is the current state, 1 enabled, 0 disabled

	 static const Command_t REPLY_COMMAND_WAS_SENT = MAKE_COMMAND_ID(4 );//!< Reply to a @c CMD_RAW or @c CMD_DIRECT, indicates the command was successfully sent to the device, getParameter() contains the ::DeviceCommandId_t
	 static const Command_t REPLY_COMMAND_NOT_SUPPORTED = MAKE_COMMAND_ID(5 );//!< Reply to a @c CMD_DIRECT, the requested ::DeviceCommand_t is not supported, getParameter() contains the requested ::DeviceCommand_t, getParameter2() contains the ::DeviceCommandId_t
	 static const Command_t REPLY_COMMAND_WAS_NOT_SENT = MAKE_COMMAND_ID(6 );//!< Reply to a @c CMD_RAW or @c CMD_DIRECT, indicates the command was not sent, getParameter contains the RMAP Return code (@see rmap.h), getParameter2() contains the ::DeviceCommandId_t

	 static const Command_t REPLY_COMMAND_ALREADY_SENT = MAKE_COMMAND_ID(7 );//!< Reply to a @c CMD_DIRECT, the requested ::DeviceCommand_t has already been sent to the device and not ye been answered
	 static const Command_t REPLY_WRONG_MODE_FOR_CMD = MAKE_COMMAND_ID(8 );//!< Reply to a @c CMD_RAW or @c CMD_DIRECT, indicates that the requested command can not be sent in the curent mode, getParameter() contains the DeviceHandlerCommand_t
	 static const Command_t REPLY_NO_DATA = MAKE_COMMAND_ID(9 );	//!< Reply to a CMD_RAW or @c CMD_DIRECT, indicates that the ::store_id_t was invalid, getParameter() contains the ::DeviceCommandId_t, getPrameter2() contains the error code
	 */
	static const Command_t REPLY_DIRECT_COMMAND_SENT = ActionMessage::STEP_SUCCESS; //!< Signals that a direct command was sent
	static const Command_t REPLY_RAW_COMMAND = MAKE_COMMAND_ID(0x11 ); //!< Contains a raw command sent to the Device
	static const Command_t REPLY_RAW_REPLY = MAKE_COMMAND_ID( 0x12); //!< Contains a raw reply from the Device, getParameter() is the ObjcetId of the sender, getParameter2() is a ::store_id_t containing the raw packet received
	static const Command_t REPLY_DIRECT_COMMAND_DATA = ActionMessage::DATA_REPLY;

	/**
	 * Default Destructor
	 */
	virtual ~DeviceHandlerMessage() {
	}

	static store_address_t getStoreAddress(const CommandMessage* message);
	static uint32_t getDeviceCommandId(const CommandMessage* message);
	static object_id_t getDeviceObjectId(const CommandMessage *message);
	static object_id_t getIoBoardObjectId(const CommandMessage* message);
	static uint8_t getWiretappingMode(const CommandMessage* message);

//	static void setDeviceHandlerDirectCommandMessage(CommandMessage* message,
//			DeviceCommandId_t deviceCommand,
//			store_address_t commandParametersStoreId);

	static void setDeviceHandlerDirectCommandReply(CommandMessage* message,
			object_id_t deviceObjectid,
			store_address_t commandParametersStoreId);

	static void setDeviceHandlerRawCommandMessage(CommandMessage* message,
			store_address_t rawPacketStoreId);

	static void setDeviceHandlerRawReplyMessage(CommandMessage* message,
			object_id_t deviceObjectid, store_address_t rawPacketStoreId,
			bool isCommand);

//	static void setDeviceHandlerMessage(CommandMessage* message,
//			Command_t command, DeviceCommandId_t deviceCommand,
//			store_address_t commandParametersStoreId);
//	static void setDeviceHandlerMessage(CommandMessage* message,
//			Command_t command, store_address_t rawPacketStoreId);
	static void setDeviceHandlerWiretappingMessage(CommandMessage* message,
			uint8_t wiretappingMode);
	static void setDeviceHandlerSwitchIoBoardMessage(CommandMessage* message,
			object_id_t ioBoardIdentifier);

	static void clear(CommandMessage* message);
};

#endif /* DEVICEHANDLERMESSAGE_H_ */
