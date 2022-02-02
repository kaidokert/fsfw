#ifndef FSFW_DEVICEHANDLERS_DEVICEHANDLERMESSAGE_H_
#define FSFW_DEVICEHANDLERS_DEVICEHANDLERMESSAGE_H_

#include "../action/ActionMessage.h"
#include "../ipc/CommandMessage.h"
#include "../objectmanager/SystemObjectIF.h"
#include "../storagemanager/StorageManagerIF.h"
// SHOULDDO: rework the static constructors to name the type of command
// they are building, maybe even hide setting the commandID.

/**
 * @brief   The DeviceHandlerMessage is used to send commands to classes
 *          implementing DeviceHandlerIF
 */
class DeviceHandlerMessage {
 public:
  /**
   * Instantiation forbidden. Instead, use static functions to operate
   * on messages.
   */
  DeviceHandlerMessage() = delete;
  virtual ~DeviceHandlerMessage() {}

  /**
   * These are the commands that can be sent to a DeviceHandlerBase
   */
  static const uint8_t MESSAGE_ID = messagetypes::DEVICE_HANDLER_COMMAND;
  //! Sends a raw command, setParameter is a storeId containing the
  //! raw packet to send
  static const Command_t CMD_RAW = MAKE_COMMAND_ID(1);
  //! Requests a IO-Board switch, setParameter() is the IO-Board identifier
  static const Command_t CMD_SWITCH_ADDRESS = MAKE_COMMAND_ID(3);
  //! (De)Activates the monitoring of all raw traffic in DeviceHandlers,
  //! setParameter is 0 to deactivate, 1 to activate
  static const Command_t CMD_WIRETAPPING = MAKE_COMMAND_ID(4);

  //! Signals that a direct command was sent
  static const Command_t REPLY_DIRECT_COMMAND_SENT = ActionMessage::STEP_SUCCESS;
  //! Contains a raw command sent to the Device
  static const Command_t REPLY_RAW_COMMAND = MAKE_COMMAND_ID(0x11);
  //! Contains a raw reply from the Device, getParameter() is the ObjcetId
  //! of the sender, getParameter2() is a ::store_id_t containing the
  //! raw packet received
  static const Command_t REPLY_RAW_REPLY = MAKE_COMMAND_ID(0x12);
  static const Command_t REPLY_DIRECT_COMMAND_DATA = ActionMessage::DATA_REPLY;

  static store_address_t getStoreAddress(const CommandMessage* message);
  static uint32_t getDeviceCommandId(const CommandMessage* message);
  static object_id_t getDeviceObjectId(const CommandMessage* message);
  static object_id_t getIoBoardObjectId(const CommandMessage* message);
  static uint8_t getWiretappingMode(const CommandMessage* message);

  static void setDeviceHandlerDirectCommandReply(CommandMessage* message,
                                                 object_id_t deviceObjectid,
                                                 store_address_t commandParametersStoreId);

  static void setDeviceHandlerRawCommandMessage(CommandMessage* message,
                                                store_address_t rawPacketStoreId);

  static void setDeviceHandlerRawReplyMessage(CommandMessage* message, object_id_t deviceObjectid,
                                              store_address_t rawPacketStoreId, bool isCommand);

  static void setDeviceHandlerWiretappingMessage(CommandMessage* message, uint8_t wiretappingMode);
  static void setDeviceHandlerSwitchIoBoardMessage(CommandMessage* message,
                                                   object_id_t ioBoardIdentifier);

  static void clear(CommandMessage* message);
};

#endif /* FSFW_DEVICEHANDLERS_DEVICEHANDLERMESSAGE_H_ */
