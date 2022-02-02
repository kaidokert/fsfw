#ifndef FSFW_DEVICEHANDLERS_DEVICEHANDLERIF_H_
#define FSFW_DEVICEHANDLERS_DEVICEHANDLERIF_H_

#include "../action/HasActionsIF.h"
#include "../datapoollocal/localPoolDefinitions.h"
#include "../events/Event.h"
#include "../ipc/MessageQueueSenderIF.h"
#include "../modes/HasModesIF.h"
#include "DeviceHandlerMessage.h"

/**
 * This is used to uniquely identify commands that are sent to a device
 * The values are defined in the device-specific implementations
 */
using DeviceCommandId_t = uint32_t;

/**
 * @brief 	This is the Interface used to communicate with a device handler.
 * @details Includes all expected return values, events and modes.
 *
 */
class DeviceHandlerIF {
 public:
  static const DeviceCommandId_t RAW_COMMAND_ID = -1;
  static const DeviceCommandId_t NO_COMMAND_ID = -2;

  static constexpr uint8_t TRANSITION_MODE_CHILD_ACTION_MASK = 0x20;
  static constexpr uint8_t TRANSITION_MODE_BASE_ACTION_MASK = 0x10;

  using dh_heater_request_t = uint8_t;
  using dh_thermal_state_t = int8_t;

  /**
   * @brief This is the mode the <strong>device handler</strong> is in.
   *
   * @details The mode of the device handler must not be confused with the mode the device is in.
   * The mode of the device itself is transparent to the user but related to the mode of the
   * handler. MODE_ON and MODE_OFF are included in hasModesIF.h
   */

  // MODE_ON = 0, //!< The device is powered and ready to perform operations. In this mode, no
  // commands are sent by the device handler itself, but direct commands van be commanded and will
  // be interpreted MODE_OFF = 1, //!< The device is powered off. The only command accepted in this
  // mode is a mode change to on.
  //! The device is powered on and the device handler periodically sends
  //! commands. The commands to be sent are selected by the handler
  //! according to the submode.
  static const Mode_t MODE_NORMAL = 2;
  //! The device is powered on and ready to perform operations. In this mode,
  //! raw commands can be sent. The device handler will send all replies
  //! received from the command back to the commanding object.
  static const Mode_t MODE_RAW = 3;
  //! The device is shut down but the switch could not be turned off, so the
  //! device still is powered. In this mode, only a mode change to @c MODE_OFF
  //! can be commanded, which tries to switch off the device again.
  static const Mode_t MODE_ERROR_ON = 4;
  //! This is a transitional state which can not be commanded. The device
  //! handler performs all commands to get the device in a state ready to
  //! perform commands. When this is completed, the mode changes to @c MODE_ON.
  static const Mode_t _MODE_START_UP = TRANSITION_MODE_CHILD_ACTION_MASK | 5;
  //! This is a transitional state which can not be commanded.
  //! The device handler performs all actions and commands to get the device
  //! shut down. When the device is off, the mode changes to @c MODE_OFF.
  //! It is possible to set the mode to _MODE_SHUT_DOWN to use the to off
  //! transition if available.
  static const Mode_t _MODE_SHUT_DOWN = TRANSITION_MODE_CHILD_ACTION_MASK | 6;
  //! It is possible to set the mode to _MODE_TO_ON to use the to on
  //! transition if available.
  static const Mode_t _MODE_TO_ON = TRANSITION_MODE_CHILD_ACTION_MASK | HasModesIF::MODE_ON;
  //! It is possible to set the mode to _MODE_TO_RAW to use the to raw
  //! transition if available.
  static const Mode_t _MODE_TO_RAW = TRANSITION_MODE_CHILD_ACTION_MASK | MODE_RAW;
  //! It is possible to set the mode to _MODE_TO_NORMAL to use the to normal
  //! transition if available.
  static const Mode_t _MODE_TO_NORMAL = TRANSITION_MODE_CHILD_ACTION_MASK | MODE_NORMAL;
  //! This is a transitional state which can not be commanded.
  //! The device is shut down and ready to be switched off.
  //! After the command to set the switch off has been sent,
  //! the mode changes to @c MODE_WAIT_OFF
  static const Mode_t _MODE_POWER_DOWN = TRANSITION_MODE_BASE_ACTION_MASK | 1;
  //! This is a transitional state which can not be commanded. The device
  //! will be switched on in this state. After the command to set the switch
  //! on has been sent, the mode changes to @c MODE_WAIT_ON.
  static const Mode_t _MODE_POWER_ON = TRANSITION_MODE_BASE_ACTION_MASK | 2;
  //! This is a transitional state which can not be commanded. The switch has
  //! been commanded off and the handler waits for it to be off.
  //! When the switch is off, the mode changes to @c MODE_OFF.
  static const Mode_t _MODE_WAIT_OFF = TRANSITION_MODE_BASE_ACTION_MASK | 3;
  //! This is a transitional state which can not be commanded. The switch
  //! has been commanded on and the handler waits for it to be on.
  //! When the switch is on, the mode changes to @c MODE_TO_ON.
  static const Mode_t _MODE_WAIT_ON = TRANSITION_MODE_BASE_ACTION_MASK | 4;
  //! This is a transitional state which can not be commanded. The switch has
  //! been commanded off and is off now. This state is only to do an RMAP
  //! cycle once more where the doSendRead() function will set the mode to
  //! MODE_OFF. The reason to do this is to get rid of stuck packets in the IO Board.
  static const Mode_t _MODE_SWITCH_IS_OFF = TRANSITION_MODE_BASE_ACTION_MASK | 5;

  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::CDH;
  static const Event DEVICE_BUILDING_COMMAND_FAILED = MAKE_EVENT(0, severity::LOW);
  static const Event DEVICE_SENDING_COMMAND_FAILED = MAKE_EVENT(1, severity::LOW);
  static const Event DEVICE_REQUESTING_REPLY_FAILED = MAKE_EVENT(2, severity::LOW);
  static const Event DEVICE_READING_REPLY_FAILED = MAKE_EVENT(3, severity::LOW);
  static const Event DEVICE_INTERPRETING_REPLY_FAILED = MAKE_EVENT(4, severity::LOW);
  static const Event DEVICE_MISSED_REPLY = MAKE_EVENT(5, severity::LOW);
  static const Event DEVICE_UNKNOWN_REPLY = MAKE_EVENT(6, severity::LOW);
  static const Event DEVICE_UNREQUESTED_REPLY = MAKE_EVENT(7, severity::LOW);
  //! [EXPORT] : [COMMENT] Indicates a SW bug in child class.
  static const Event INVALID_DEVICE_COMMAND = MAKE_EVENT(8, severity::LOW);
  static const Event MONITORING_LIMIT_EXCEEDED = MAKE_EVENT(9, severity::LOW);
  static const Event MONITORING_AMBIGUOUS = MAKE_EVENT(10, severity::HIGH);

  static const uint8_t INTERFACE_ID = CLASS_ID::DEVICE_HANDLER_IF;

  // Standard codes used when building commands.
  static const ReturnValue_t NO_COMMAND_DATA =
      MAKE_RETURN_CODE(0xA0);  //!< If no command data was given when expected.
  static const ReturnValue_t COMMAND_NOT_SUPPORTED =
      MAKE_RETURN_CODE(0xA1);  //!< Command ID not in commandMap. Checked in DHB
  static const ReturnValue_t COMMAND_ALREADY_SENT =
      MAKE_RETURN_CODE(0xA2);  //!< Command was already executed. Checked in DHB
  static const ReturnValue_t COMMAND_WAS_NOT_SENT = MAKE_RETURN_CODE(0xA3);
  static const ReturnValue_t CANT_SWITCH_ADDRESS = MAKE_RETURN_CODE(0xA4);
  static const ReturnValue_t WRONG_MODE_FOR_COMMAND = MAKE_RETURN_CODE(0xA5);
  static const ReturnValue_t TIMEOUT = MAKE_RETURN_CODE(0xA6);
  static const ReturnValue_t BUSY = MAKE_RETURN_CODE(0xA7);
  //!< Used to indicate that this is a command-only command.
  static const ReturnValue_t NO_REPLY_EXPECTED = MAKE_RETURN_CODE(0xA8);
  static const ReturnValue_t NON_OP_TEMPERATURE = MAKE_RETURN_CODE(0xA9);
  static const ReturnValue_t COMMAND_NOT_IMPLEMENTED = MAKE_RETURN_CODE(0xAA);

  // Standard codes used in scanForReply
  static const ReturnValue_t CHECKSUM_ERROR = MAKE_RETURN_CODE(0xB0);
  static const ReturnValue_t LENGTH_MISSMATCH = MAKE_RETURN_CODE(0xB1);
  static const ReturnValue_t INVALID_DATA = MAKE_RETURN_CODE(0xB2);
  static const ReturnValue_t PROTOCOL_ERROR = MAKE_RETURN_CODE(0xB3);

  // Standard codes used in  interpretDeviceReply
  static const ReturnValue_t DEVICE_DID_NOT_EXECUTE =
      MAKE_RETURN_CODE(0xC0);  // the device reported, that it did not execute the command
  static const ReturnValue_t DEVICE_REPORTED_ERROR = MAKE_RETURN_CODE(0xC1);
  static const ReturnValue_t UNKNOWN_DEVICE_REPLY =
      MAKE_RETURN_CODE(0xC2);  // the deviceCommandId reported by scanforReply is unknown
  static const ReturnValue_t DEVICE_REPLY_INVALID = MAKE_RETURN_CODE(
      0xC3);  // syntax etc is correct but still not ok, eg parameters where none are expected

  // Standard codes used in buildCommandFromCommand
  static const ReturnValue_t INVALID_COMMAND_PARAMETER = MAKE_RETURN_CODE(0xD0);
  static const ReturnValue_t INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS = MAKE_RETURN_CODE(0xD1);

  /**
   * Communication action that will be executed.
   *
   * This is used by the child class to tell the base class what to do.
   */
  enum CommunicationAction : uint8_t {
    PERFORM_OPERATION,
    SEND_WRITE,  //!< Send write
    GET_WRITE,   //!< Get write
    SEND_READ,   //!< Send read
    GET_READ,    //!< Get read
    NOTHING      //!< Do nothing.
  };

  static constexpr uint32_t DEFAULT_THERMAL_SET_ID = sid_t::INVALID_SET_ID - 1;

  static constexpr lp_id_t DEFAULT_THERMAL_STATE_POOL_ID = localpool::INVALID_LPID - 2;
  static constexpr lp_id_t DEFAULT_THERMAL_HEATING_REQUEST_POOL_ID = localpool::INVALID_LPID - 1;

  /**
   * Default Destructor
   */
  virtual ~DeviceHandlerIF() {}

  /**
   * This MessageQueue is used to command the device handler.
   * @return the id of the MessageQueue
   */
  virtual MessageQueueId_t getCommandQueue() const = 0;
};

#endif /* FSFW_DEVICEHANDLERS_DEVICEHANDLERIF_H_ */
