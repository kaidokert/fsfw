#ifndef FSFW_DEVICEHANDLERS_DEVICEHANDLERBASE_H_
#define FSFW_DEVICEHANDLERS_DEVICEHANDLERBASE_H_

#include <map>

#include "DeviceCommunicationIF.h"
#include "DeviceHandlerFailureIsolation.h"
#include "DeviceHandlerIF.h"
#include "DeviceHandlerThermalSet.h"
#include "fsfw/action/ActionHelper.h"
#include "fsfw/action/HasActionsIF.h"
#include "fsfw/datapool/PoolVariableIF.h"
#include "fsfw/datapoollocal/HasLocalDataPoolIF.h"
#include "fsfw/datapoollocal/LocalDataPoolManager.h"
#include "fsfw/health/HealthHelper.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/modes/HasModesIF.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/parameters/ParameterHelper.h"
#include "fsfw/power/PowerSwitchIF.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/serviceinterface/serviceInterfaceDefintions.h"
#include "fsfw/tasks/ExecutableObjectIF.h"
#include "fsfw/tasks/PeriodicTaskIF.h"

namespace Factory {
void setStaticFrameworkObjectIds();
}

class StorageManagerIF;

/**
 * @defgroup devices Devices
 * Contains all devices and the DeviceHandlerBase class.
 */

/**
 * @brief This is the abstract base class for device handlers.
 * @details
 * Documentation: Dissertation Baetz p.138,139, p.141-149
 *
 * It features handling of @link DeviceHandlerIF::Mode_t Modes @endlink,
 * communication with physical devices, using the
 * @link DeviceCommunicationIF @endlink, and communication with commanding
 * objects. It inherits SystemObject and thus can be created by the
 * ObjectManagerIF.
 *
 * This class uses the opcode of ExecutableObjectIF to perform a
 * step-wise execution. For each step a different action is selected and
 * executed. Currently, the device handler base performs a 4-step
 * execution related to 4 communication steps (based on RMAP).
 * NOTE: RMAP is a standard which is used for Flying Laptop.
 * RMAP communication is not mandatory for projects implementing the FSFW.
 * However, the communication principles are similar to RMAP as there are
 * two write and two send calls involved.
 *
 * Device handler instances should extend this class and implement the abstract
 * functions. Components and drivers can send so called cookies which are used
 * for communication and contain information about the communcation (e.g. slave
 * address for I2C or RMAP structs).
 * The following abstract methods must be implemented by a device handler:
 *  1. doStartUp()
 *  2. doShutDown()
 *  3. buildTransitionDeviceCommand()
 *  4. buildNormalDeviceCommand()
 *  5. buildCommandFromCommand()
 *  6. fillCommandAndReplyMap()
 *  7. scanForReply()
 *  8. interpretDeviceReply()
 *
 * Other important virtual methods with a default implementation
 * are the getTransitionDelayMs() function and the getSwitches() function.
 * If a transition to MODE_ON is desired without commanding, override the
 * intialize() function and call setMode(_MODE_START_UP) before calling
 * DeviceHandlerBase::initialize().
 *
 * @ingroup devices
 */
class DeviceHandlerBase : public DeviceHandlerIF,
                          public ExecutableObjectIF,
                          public SystemObject,
                          public HasModesIF,
                          public HasHealthIF,
                          public HasActionsIF,
                          public ReceivesParameterMessagesIF,
                          public HasLocalDataPoolIF {
  friend void(Factory::setStaticFrameworkObjectIds)();

 public:
  /**
   * The constructor passes the objectId to the SystemObject().
   *
   * @param setObjectId the ObjectId to pass to the SystemObject() Constructor
   * @param deviceCommuncation Communcation Interface object which is used
   * to implement communication functions
   * @param comCookie This object will be passed to the communication inter-
   * face and can contain user-defined information about the communication.
   * @param fdirInstance
   * @param cmdQueueSize
   */
  DeviceHandlerBase(object_id_t setObjectId, object_id_t deviceCommunication, CookieIF *comCookie,
                    FailureIsolationBase *fdirInstance = nullptr, size_t cmdQueueSize = 20);

  void setHkDestination(object_id_t hkDestination);

  /**
   * If the device handler is controlled by the FSFW thermal building blocks,
   * this function should be called to initialize all required components.
   * The device handler will then take care of creating local pool entries
   * for the device thermal state and device heating request.
   * Custom local pool IDs can be assigned as well.
   * @param thermalStatePoolId
   * @param thermalRequestPoolId
   */
  void setThermalStateRequestPoolIds(
      lp_id_t thermalStatePoolId = DeviceHandlerIF::DEFAULT_THERMAL_STATE_POOL_ID,
      lp_id_t thermalRequestPoolId = DeviceHandlerIF::DEFAULT_THERMAL_HEATING_REQUEST_POOL_ID,
      uint32_t thermalSetId = DeviceHandlerIF::DEFAULT_THERMAL_SET_ID);
  /**
   * @brief   Helper function to ease device handler development.
   * This will instruct the transition to MODE_ON immediately
   * (leading to doStartUp() being called for the transition to the ON mode),
   * so external mode commanding is not necessary anymore.
   *
   * This has to be called before the task is started!
   * (e.g. in the task factory). This is only a helper function for
   * development. Regular mode commanding should be performed by commanding
   * the AssemblyBase or Subsystem objects resposible for the device handler.
   */
  void setStartUpImmediately();

  /**
   * @brief 	This function is the device handler base core component and is
   * 			called periodically.
   * @details
   * General sequence, showing where abstract virtual functions are called:
   * If the State is SEND_WRITE:
   *   1. Set the cookie state to COOKIE_UNUSED and read the command queue
   *   2. Handles Device State Modes by calling doStateMachine().
   *      This function calls callChildStatemachine() which calls the
   *      abstract functions doStartUp() and doShutDown()
   *   3. Check switch states by calling checkSwitchStates()
   *   4. Decrements counter for timeout of replies by calling
   *      decrementDeviceReplyMap()
   *   5. Performs FDIR check for failures
   *   6. If the device mode is MODE_OFF, return returnvalue::OK.
   *      Otherwise, perform the Action property and performs depending
   *      on value specified by input value counter (incremented in PST).
   *      The child class tells base class what to do by setting this value.
   *     - SEND_WRITE: Send data or commands to device by calling
   *       doSendWrite() which calls sendMessage function
   *       of #communicationInterface
   *     	 and calls buildInternalCommand if the cookie state is COOKIE_UNUSED
   *     - GET_WRITE: Get ackknowledgement for sending by calling doGetWrite()
   *       which calls getSendSuccess of #communicationInterface.
   *       Calls abstract functions scanForReply() and interpretDeviceReply().
   *     - SEND_READ: Request reading data from device by calling doSendRead()
   *       which calls requestReceiveMessage of #communcationInterface
   *     - GET_READ: Access requested reading data by calling doGetRead()
   *       which calls readReceivedMessage of #communicationInterface
   * @param counter Specifies which Action to perform
   * @return returnvalue::OK for successful execution
   */
  virtual ReturnValue_t performOperation(uint8_t counter) override;

  /**
   * @brief  Initializes the device handler
   * @details
   * Initialize Device Handler as system object and
   * initializes all important helper classes.
   * Calls fillCommandAndReplyMap().
   * @return
   */
  virtual ReturnValue_t initialize() override;

  /**
   * @brief   Intialization steps performed after all tasks have been created.
   *          This function will be called by the executing task.
   * @return
   */
  virtual ReturnValue_t initializeAfterTaskCreation() override;

  /** Destructor. */
  virtual ~DeviceHandlerBase();

  /**
   * Implementation of ExecutableObjectIF function
   * Used to setup the reference of the task, that executes this component
   * @param task_ Pointer to the taskIF of this task
   */
  virtual void setTaskIF(PeriodicTaskIF *task_) override;
  virtual MessageQueueId_t getCommandQueue(void) const override;

  /** Explicit interface implementation of getObjectId */
  virtual object_id_t getObjectId() const override;

  /**
   * @param parentQueueId
   */
  virtual void setParentQueue(MessageQueueId_t parentQueueId);

  /** @brief  Implementation required for HasActionIF */
  ReturnValue_t executeAction(ActionId_t actionId, MessageQueueId_t commandedBy,
                              const uint8_t *data, size_t size) override;

  Mode_t getTransitionSourceMode() const;
  Submode_t getTransitionSourceSubMode() const;
  virtual void getMode(Mode_t *mode, Submode_t *submode);
  HealthState getHealth();
  ReturnValue_t setHealth(HealthState health);
  virtual ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueId,
                                     ParameterWrapper *parameterWrapper,
                                     const ParameterWrapper *newValues,
                                     uint16_t startAtIndex) override;

 protected:
  /**
   * @brief 	This is used to let the child class handle the transition from
   * 			mode @c _MODE_START_UP to @c MODE_ON
   * @details
   * It is only called when the device handler is in mode @c _MODE_START_UP.
   * That means, the device switch(es) are already set to on.
   * Device handler commands are read and can be handled by the child class.
   * If the child class handles a command, it should also send
   * an reply accordingly.
   * If an Command is not handled (ie #DeviceHandlerCommand is not @c CMD_NONE,
   * the base class handles rejecting the command and sends a reply.
   * The replies for mode transitions are handled by the base class.
   *
   *  - If the device is started and ready for operation, the mode should be
   *    set to MODE_ON. It is possible to set the mode to _MODE_TO_ON to
   *    use the to on transition if available.
   *  - If the power-up fails, the mode should be set to _MODE_POWER_DOWN
   *    which will lead to the device being powered off.
   *  - If the device does not change the mode, the mode will be changed
   *    to _MODE_POWER_DOWN, after the timeout (from getTransitionDelay())
   *    has passed.
   *
   * #transitionFailure can be set to a failure code indicating the reason
   * for a failed transition
   */
  virtual void doStartUp() = 0;
  /**
   * @brief 	This is used to let the child class handle the transition
   * 			from mode @c _MODE_SHUT_DOWN to @c _MODE_POWER_DOWN
   * @details
   * It is only called when the device handler is in mode @c _MODE_SHUT_DOWN.
   * Device handler commands are read and can be handled by the child class.
   * If the child class handles a command, it should also send an reply
   * accordingly.
   * If an Command is not handled (ie #DeviceHandlerCommand is not
   * @c CMD_NONE, the base class handles rejecting the command and sends a
   * reply. The replies for mode transitions are handled by the base class.
   *
   *  - If the device ready to be switched off,
   *    the mode should be set to _MODE_POWER_DOWN.
   *  - If the device should not be switched off, the mode can be changed to
   *     _MODE_TO_ON (or MODE_ON if no transition is needed).
   *  - If the device does not change the mode, the mode will be changed to
   *    _MODE_POWER_DOWN, when the timeout (from getTransitionDelay())
   *    has passed.
   *
   * #transitionFailure can be set to a failure code indicating the reason
   * for a failed transition
   */
  virtual void doShutDown() = 0;

  /* Command handling */
  /**
   * Build the device command to send for normal mode.
   *
   * This is only called in @c MODE_NORMAL. If multiple submodes for
   * @c MODE_NORMAL are supported, different commands can built,
   * depending on the submode.
   *
   * #rawPacket and #rawPacketLen must be set by this method to the
   * packet to be sent. If variable command frequence is required, a counter
   * can be used and the frequency in the reply map has to be set manually
   * by calling updateReplyMap().
   *
   * @param[out] id the device command id that has been built
   * @return
   *  - @c returnvalue::OK to send command after setting #rawPacket and
   *    #rawPacketLen.
   *  - @c NOTHING_TO_SEND when no command is to be sent.
   *  - Anything else triggers an even with the returnvalue as a parameter.
   */
  virtual ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t *id) = 0;
  /**
   * Build the device command to send for a transitional mode.
   *
   * This is only called in @c _MODE_TO_NORMAL, @c _MODE_TO_ON, @c _MODE_TO_RAW,
   * @c _MODE_START_UP and @c _MODE_SHUT_DOWN. So it is used by doStartUp()
   * and doShutDown() as well as doTransition(), by setting those
   * modes in the respective functions.
   *
   * A good idea is to implement a flag indicating a command has to be built
   * and a variable containing the command number to be built
   * and filling them in doStartUp(), doShutDown() and doTransition() so no
   * modes have to be checked here.
   *
   * #rawPacket and #rawPacketLen must be set by this method to the
   * packet to be sent.
   *
   * @param[out] id the device command id built
   * @return
   *    - @c returnvalue::OK when a command is to be sent
   *    - @c NOTHING_TO_SEND when no command is to be sent
   *    - Anything else triggers an even with the returnvalue as a parameter
   */
  virtual ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t *id) = 0;
  /**
   * @brief 	Build a device command packet from data supplied by a direct
   *          command (PUS Service 8)
   * @details
   * This will be called if an functional command via PUS Service 8 is received and is
   * the primary interface for functional command instead of #executeAction for users. The
   * supplied ActionId_t action ID will be converted to a DeviceCommandId_t command ID after
   * an internal check whether the action ID is a key in the device command map.
   *
   * #rawPacket and #rawPacketLen should be set by this method to the packet to be sent.
   * The existence of the command in the command map and the command size check against 0 are
   * done by the base class.
   *
   * @param deviceCommand     The command to build, already checked against deviceCommandMap
   * @param commandData       Pointer to the data from the direct command
   * @param commandDataLen    Length of commandData
   * @return
   *  - @c returnvalue::OK to send command after #rawPacket and #rawPacketLen
   *       have been set.
   *  - @c HasActionsIF::EXECUTION_COMPLETE to generate a finish reply immediately. This can
   *       be used if no reply is expected
   *  - Anything else triggers an event with the return code as a parameter as well as a
   *    step reply failed with the return code
   */
  virtual ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand,
                                                const uint8_t *commandData,
                                                size_t commandDataLen) = 0;

  /* Reply handling */
  /**
   * @brief   Scans a buffer for a valid reply.
   * @details
   * This is used by the base class to check the data received for valid packets.
   * It only checks if a valid packet starts at @c start.
   * It also only checks the structural validy of the packet,
   * e.g. checksums lengths and protocol data. No information check is done,
   * e.g. range checks etc.
   *
   * Errors should be reported directly, the base class does NOT report any
   * errors based on the return value of this function.
   *
   * @param start start of remaining buffer to be scanned
   * @param len length of remaining buffer to be scanned
   * @param[out] foundId the id of the data found in the buffer.
   * @param[out] foundLen length of the data found. Is to be set in function,
   *                       buffer is scanned at previous position + foundLen.
   * @return
   *  - @c returnvalue::OK a valid packet was found at @c start, @c foundLen is valid
   *  - @c returnvalue::FAILED no reply could be found starting at @c start,
   *    implies @c foundLen is not valid, base class will call scanForReply()
   *    again with ++start
   *  - @c DeviceHandlerIF::INVALID_DATA a packet was found but it is invalid,
   *    e.g. checksum error, implies @c foundLen is valid, can be used to
   *    skip some bytes
   *  - @c DeviceHandlerIF::LENGTH_MISSMATCH @c len is invalid
   *  - @c DeviceHandlerIF::IGNORE_REPLY_DATA Ignore this specific part of
   *    the packet
   *  - @c DeviceHandlerIF::IGNORE_FULL_PACKET Ignore the packet
   *  - @c APERIODIC_REPLY if a valid reply is received that has not been
   *    requested by a command, but should be handled anyway
   *    (@see also fillCommandAndCookieMap() )
   */
  virtual ReturnValue_t scanForReply(const uint8_t *start, size_t len, DeviceCommandId_t *foundId,
                                     size_t *foundLen) = 0;
  /**
   * @brief   Interpret a reply from the device.
   * @details
   * This is called after scanForReply() found a valid packet, it can be
   * assumed that the length and structure is valid.
   * This routine extracts the data from the packet into a DataSet and then
   * calls handleDeviceTM(), which either sends a TM packet or stores the
   * data in the DataPool depending on whether it was an external command.
   * No packet length is given, as it should be defined implicitly by the id.
   *
   * @param id the id found by scanForReply()
   * @param packet
   * @return
   *     - @c returnvalue::OK when the reply was interpreted.
   *     - @c IGNORE_REPLY_DATA Ignore the reply and don't reset reply cycle
   *          counter.
   *     - @c returnvalue::FAILED when the reply could not be interpreted,
   *     e.g. logical errors or range violations occurred
   */
  virtual ReturnValue_t interpretDeviceReply(DeviceCommandId_t id, const uint8_t *packet) = 0;
  MessageQueueId_t getCommanderQueueId(DeviceCommandId_t replyId) const;
  /**
   * Helper function to get pending command. This is useful for devices
   * like SPI sensors to identify the last sent command.
   * This only returns the command sent in the last SEND_WRITE cycle.
   * @return
   */
  DeviceCommandId_t getPendingCommand() const;

  /* Specifying commands and replies */
  /**
   * @brief   Fill the #DeviceCommandMap and #DeviceReplyMap called by the #initialize
   *          of the base class
   * @details
   * This is used to let the base class know which replies are expected.
   * There are different scenarios regarding this:
   *
   *  - "Normal" commands. These are commands, that trigger a direct reply
   *    from the device. In this case, the id of the command should be added
   *    to the command map with a commandData_t where maxDelayCycles is set
   *    to the maximum expected number of PST cycles the reply will take.
   *    Then, scanForReply returns the id of the command and the base class
   *    can handle time-out and missing replies.
   *
   *  - Periodic, unrequested replies. These are replies that, once enabled,
   *    are sent by the device on its own in a defined interval.
   *    In this case, the id of the reply or a placeholder id should be added
   *    to the deviceCommandMap with a commandData_t where maxDelayCycles is
   *    set to the maximum expected number of PST cycles between two replies
   *    (also a tolerance should be added, as an FDIR message will be
   *    generated if it is missed).
   *    From then on, the base class handles the reception.
   *    Then, scanForReply returns the id of the reply or the placeholder id
   *    and the base class will take care of checking that all replies are
   *    received and the interval is correct.
   *
   *  - Aperiodic, unrequested replies. These are replies that are sent
   *    by the device without any preceding command and not in a defined
   *    interval. These are not entered in the deviceCommandMap but
   *    handled by returning @c APERIODIC_REPLY in scanForReply().
   */
  virtual void fillCommandAndReplyMap() = 0;
  /**
   * This is a helper method to facilitate inserting entries in the command map.
   * @param deviceCommand	Identifier of the command to add.
   * @param maxDelayCycles The maximum number of delay cycles the command
   * waits until it times out.
   * @param replyLen Will be supplied to the requestReceiveMessage call of
   * the communication interface.
   * @param periodic	Indicates if the command is periodic (i.e. it is sent
   * by the device repeatedly without request) or not. Default is aperiodic (0).
   * Please note that periodic replies are disabled by default. You can enable them with
   * #updatePeriodicReply
   * @param countdown Instead of using maxDelayCycles to timeout a device reply it is also possible
   *                  to provide a pointer to a Countdown object which will signal the timeout
   *                  when expired
   * @return	- @c returnvalue::OK when the command was successfully inserted,
   *          - @c returnvalue::FAILED else.
   */
  ReturnValue_t insertInCommandAndReplyMap(DeviceCommandId_t deviceCommand, uint16_t maxDelayCycles,
                                           LocalPoolDataSetBase *replyDataSet = nullptr,
                                           size_t replyLen = 0, bool periodic = false,
                                           bool hasDifferentReplyId = false,
                                           DeviceCommandId_t replyId = 0,
                                           Countdown *countdown = nullptr);
  /**
   * @brief 	This is a helper method to insert replies in the reply map.
   * @param deviceCommand	Identifier of the reply to add.
   * @param maxDelayCycles The maximum number of delay cycles the reply waits
   * until it times out.
   * @param periodic	Indicates if the command is periodic (i.e. it is sent
   * by the device repeatedly without request) or not. Default is aperiodic (0).
   * Please note that periodic replies are disabled by default. You can enable them with
   * #updatePeriodicReply
   * @param countdown Instead of using maxDelayCycles to timeout a device reply it is also possible
   *                  to provide a pointer to a Countdown object which will signal the timeout
   *                  when expired
   * @return	- @c returnvalue::OK when the command was successfully inserted,
   *          - @c returnvalue::FAILED else.
   */
  ReturnValue_t insertInReplyMap(DeviceCommandId_t deviceCommand, uint16_t maxDelayCycles,
                                 LocalPoolDataSetBase *dataSet = nullptr, size_t replyLen = 0,
                                 bool periodic = false, Countdown *countdown = nullptr);

  /**
   * @brief   A simple command to add a command to the commandList.
   * @param deviceCommand The command to add
   * @return - @c returnvalue::OK when the command was successfully inserted,
   *         - @c returnvalue::FAILED else.
   */
  ReturnValue_t insertInCommandMap(DeviceCommandId_t deviceCommand,
                                   bool useAlternativeReply = false,
                                   DeviceCommandId_t alternativeReplyId = 0);

  /**
   * Enables a periodic reply for a given command. It sets to delay cycles to the specified
   * maximum delay cycles for a given reply ID if enabled or to 0 if disabled.
   * @param enable Specify whether to enable or disable a given periodic reply
   * @return
   */
  ReturnValue_t updatePeriodicReply(bool enable, DeviceCommandId_t deviceReply);

  /**
   * @brief   This function returns the reply length of the next reply to read.
   *
   * @param deviceCommand The command which triggered the device reply.
   *
   * @details The default implementation assumes only one reply is triggered by the command. In
   *          case the command triggers multiple replies (e.g. one acknowledgment, one data,
   *          and one execution status reply), this function can be overwritten to get the
   *          reply length of the next reply to read.
   */
  virtual size_t getNextReplyLength(DeviceCommandId_t deviceCommand);

  /**
   * @brief 	This is a helper method to facilitate updating entries in the reply map.
   * @param deviceCommand	Identifier of the reply to update.
   * @param delayCycles The current number of delay cycles to wait. As stated in
   * #fillCommandAndReplyMap, to disable periodic commands, this is set to zero.
   * @param maxDelayCycles The maximum number of delay cycles the reply waits
   * until it times out. By passing 0 the entry remains untouched.
   * @param periodic Indicates if the command is periodic (i.e. it is sent
   * by the device repeatedly without request) or not. Default is aperiodic (0).
   * Warning: The setting always overrides the value that was entered in the map.
   * @return - @c returnvalue::OK when the command was successfully inserted,
   *         - @c returnvalue::FAILED else.
   */
  ReturnValue_t updateReplyMapEntry(DeviceCommandId_t deviceReply, uint16_t delayCycles,
                                    uint16_t maxDelayCycles, bool periodic = false);
  /**
   * @brief   Can be used to set the dataset corresponding to a reply ID manually.
   * @details
   * Used by the local data pool manager.
   */
  ReturnValue_t setReplyDataset(DeviceCommandId_t replyId, LocalPoolDataSetBase *dataset);

  /**
   * Get the time needed to transit from modeFrom to modeTo.
   *
   * Used for the following transitions:
   * modeFrom -> modeTo:
   * MODE_ON -> [MODE_ON, MODE_NORMAL, MODE_RAW, _MODE_POWER_DOWN]
   * MODE_NORMAL -> [MODE_ON, MODE_NORMAL, MODE_RAW, _MODE_POWER_DOWN]
   * MODE_RAW -> [MODE_ON, MODE_NORMAL, MODE_RAW, _MODE_POWER_DOWN]
   * _MODE_START_UP -> MODE_ON (do not include time to set the switches,
   * the base class got you covered)
   *
   * The default implementation returns 0 !
   * @param modeFrom
   * @param modeTo
   * @return time in ms
   */
  virtual uint32_t getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) = 0;

  /* Functions used by the local data pool manager */
  /**
   * This function is used to initialize the local housekeeping pool
   * entries. The default implementation leaves the pool empty.
   * @param localDataPoolMap
   * @return
   */
  virtual ReturnValue_t initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
                                                LocalDataPoolManager &poolManager) override;
  /**
   * @brief   Set all datapool variables that are update periodically in
   *          normal mode invalid
   * @details
   * The default implementation will set all datasets which have been added
   * in #fillCommandAndReplyMap to invalid. It will also set all pool
   * variables inside the dataset to invalid. The user can override this
   * method optionally.
   */
  virtual void setNormalDatapoolEntriesInvalid();
  /**
   * @brief   Get the dataset handle for a given SID.
   * @details
   * The default implementation will use the deviceCommandMap to look for the corresponding
   * dataset handle. The user can override this function if this is not desired.
   * @param sid
   * @return
   */
  virtual LocalPoolDataSetBase *getDataSetHandle(sid_t sid) override;

  /* HasModesIF overrides */
  virtual void startTransition(Mode_t mode, Submode_t submode) override;
  virtual void setToExternalControl() override;
  virtual void announceMode(bool recursive) override;
  /**
   * @brief   Set the device handler mode
   * @details
   * Sets #timeoutStart with every call Also sets #transitionTargetMode if necessary so
   * transitional states can be entered from everywhere without breaking the state machine
   * (which relies on a correct #transitionTargetMode).
   * The submode is left unchanged.
   *
   * @param newMode
   */
  void setMode(Mode_t newMode);
  /**
   * @overload
   * @param submode
   */
  void setMode(Mode_t newMode, Submode_t submode);
  /**
   * @brief   Should be implemented properly by child class.
   * @param mode
   * @param submode
   * @return
   *    - @c returnvalue::OK if valid
   *    - @c returnvalue::FAILED if invalid
   */
  virtual ReturnValue_t isModeCombinationValid(Mode_t mode, Submode_t submode);
  /**
   * @brief   Notify child about mode change.
   * @details
   * Can be overriden to be used like a callback.
   */
  virtual void modeChanged();

  /* Power handling functions */
  /**
   * Return the switches connected to the device.
   *
   * The default implementation returns one switch set in the ctor.
   *
   * @param[out] switches pointer to an array of switches
   * @param[out] numberOfSwitches length of returned array
   * @return
   *      - @c returnvalue::OK if the parameters were set
   *      - @c returnvalue::FAILED if no switches exist
   */
  virtual ReturnValue_t getSwitches(const uint8_t **switches, uint8_t *numberOfSwitches);

  /**
   * @brief   Helper function to report a missed reply
   * @details Can be overwritten by children to act on missed replies or to fake reporting Id.
   * @param id of the missed reply
   */
  virtual void missedReply(DeviceCommandId_t id);

  /* Miscellaneous functions */
  /**
   * @brief 	Hook function for child handlers which is called once per
   * 			performOperation(). Default implementation is empty.
   */
  virtual void performOperationHook();
  /**
   * @brief   Can be implemented by child handler to
   *          perform debugging
   * @details Example: Calling this in performOperation
   *          to track values like mode.
   * @param positionTracker Provide the child handler a way to know
   * where the debugInterface was called
   * @param objectId Provide the child handler object Id to
   * specify actions for spefic devices
   * @param parameter Supply a parameter of interest
   * Please delete all debugInterface calls in DHB after debugging is finished !
   */
  virtual void debugInterface(uint8_t positionTracker = 0, object_id_t objectId = 0,
                              uint32_t parameter = 0);

 protected:
  static const uint8_t INTERFACE_ID = CLASS_ID::DEVICE_HANDLER_BASE;

  static const ReturnValue_t INVALID_CHANNEL = MAKE_RETURN_CODE(0xA0);
  /* Return codes for scanForReply */
  //! This is used to specify for replies from a device which are not replies to requests
  static const ReturnValue_t APERIODIC_REPLY = MAKE_RETURN_CODE(0xB0);
  //! Ignore parts of the received packet
  static const ReturnValue_t IGNORE_REPLY_DATA = MAKE_RETURN_CODE(0xB1);
  //! Ignore full received packet
  static const ReturnValue_t IGNORE_FULL_PACKET = MAKE_RETURN_CODE(0xB2);
  /* Return codes for command building */
  //! Return this if no command sending in required
  static const ReturnValue_t NOTHING_TO_SEND = MAKE_RETURN_CODE(0xC0);
  static const ReturnValue_t COMMAND_MAP_ERROR = MAKE_RETURN_CODE(0xC2);
  // Return codes for getSwitches */
  static const ReturnValue_t NO_SWITCH = MAKE_RETURN_CODE(0xD0);
  /* Mode handling error Codes */
  static const ReturnValue_t CHILD_TIMEOUT = MAKE_RETURN_CODE(0xE0);
  static const ReturnValue_t SWITCH_FAILED = MAKE_RETURN_CODE(0xE1);

  static const MessageQueueId_t NO_COMMANDER = MessageQueueIF::NO_QUEUE;

  //! Pointer to the raw packet that will be sent.
  uint8_t *rawPacket = nullptr;
  //! Size of the #rawPacket.
  size_t rawPacketLen = 0;

  /**
   * The mode the device handler is currently in.
   * This should never be changed directly but only with setMode()
   */
  Mode_t mode;
  /**
   * The submode the device handler is currently in.
   * This should never be changed directly but only with setMode()
   */
  Submode_t submode;

  /** This is the counter value from performOperation(). */
  uint8_t pstStep = 0;
  uint8_t lastStep = 0;
  uint32_t pstIntervalMs = 0;

  /**
   * Wiretapping flag:
   *
   * indicates either that all raw messages to and from the device should be
   * sent to #defaultRawReceiver
   * or that all device TM should be downlinked to #defaultRawReceiver.
   */
  enum WiretappingMode { OFF = 0, RAW = 1, TM = 2 } wiretappingMode;
  /**
   * @brief 	A message queue that accepts raw replies
   *
   * Statically initialized in initialize() to a configurable object.
   * Used when there is no method of finding a recipient, ie raw mode and
   * reporting erroneous replies
   */
  MessageQueueId_t defaultRawReceiver = MessageQueueIF::NO_QUEUE;
  store_address_t storedRawData;

  /**
   * @brief 	The message queue which wants to read all raw traffic
   * If #isWiretappingActive all raw communication from and to the device
   * will be sent to this queue
   */
  MessageQueueId_t requestedRawTraffic = 0;

  /**
   * Pointer to the IPCStore.
   * This caches the pointer received from the objectManager in the constructor.
   */
  StorageManagerIF *IPCStore = nullptr;
  /** The comIF object ID is cached for the intialize() function */
  object_id_t deviceCommunicationId;
  /** Communication object used for device communication */
  DeviceCommunicationIF *communicationInterface = nullptr;
  /** Cookie used for communication */
  CookieIF *comCookie;

  /* Health helper for HasHealthIF */
  HealthHelper healthHelper;
  /* Mode helper for HasModesIF */
  ModeHelper modeHelper;
  /* Parameter helper for ReceivesParameterMessagesIF */
  ParameterHelper parameterHelper;
  /* Action helper for HasActionsIF */
  ActionHelper actionHelper;
  /* Housekeeping Manager */
  LocalDataPoolManager poolManager;

  /**
   *  @brief Information about commands
   */
  struct DeviceCommandInfo {
    //! Indicates if the command is already executing.
    bool isExecuting;
    //! Dynamic value to indicate how many replies are expected.
    //! Inititated with 0.
    uint8_t expectedReplies;
    //! if this is != NO_COMMANDER, DHB was commanded externally and shall
    //! report everything to commander.
    MessageQueueId_t sendReplyTo;
    bool useAlternativeReplyId;
    DeviceCommandId_t alternativeReplyId;
  };
  using DeviceCommandMap = std::map<DeviceCommandId_t, DeviceCommandInfo>;
  /**
   * Information about commands
   */
  DeviceCommandMap deviceCommandMap;

  /**
   * @brief Information about expected replies
   * This is used to keep track of pending replies.
   */
  struct DeviceReplyInfo {
    //! The maximum number of cycles the handler should wait for a reply
    //! to this command.
    uint16_t maxDelayCycles;
    //! The currently remaining cycles the handler should wait for a reply,
    //! 0 means there is no reply expected
    uint16_t delayCycles;
    size_t replyLen = 0;  //!< Expected size of the reply.
    //! if this is !=0, the delayCycles will not be reset to 0 but to
    //! maxDelayCycles
    bool periodic = false;
    //! The dataset used to access housekeeping data related to the
    //! respective device reply. Will point to a dataset held by
    //! the child handler (if one is specified)
    LocalPoolDataSetBase *dataSet = nullptr;
    //! The command that expects this reply.
    DeviceCommandMap::iterator command;
    //! Instead of using delayCycles to specify the maximum time to wait for the device reply, it
    //! is also possible specify a countdown
    Countdown *countdown = nullptr;
    //! will be set to true when reply is enabled
    bool active = false;
  };

  using DeviceReplyMap = std::map<DeviceCommandId_t, DeviceReplyInfo>;
  using DeviceReplyIter = DeviceReplyMap::iterator;
  /**
   * This map is used to check and track correct reception of all replies.
   *
   * It has multiple use:
   * - It stores the information on pending replies. If a command is sent,
   * 	 the DeviceCommandInfo.count is incremented.
   * - It is used to time-out missing replies. If a command is sent, the
   * 	 DeviceCommandInfo.DelayCycles is set to MaxDelayCycles.
   * - It is queried to check if a reply from the device can be interpreted.
   *   scanForReply() returns the id of the command a reply was found for.
   * The reply is ignored in the following cases:
   *     - No entry for the returned id was found
   *     - The deviceReplyInfo.delayCycles is == 0
   */
  DeviceReplyMap deviceReplyMap;

  //! The MessageQueue used to receive device handler commands
  //! and to send replies.
  MessageQueueIF *commandQueue = nullptr;

  DeviceHandlerThermalSet *thermalSet = nullptr;

  /**
   * Optional Error code. Can be set in doStartUp(), doShutDown() and
   * doTransition() to signal cause for Transition failure.
   */
  ReturnValue_t childTransitionFailure;

  /** Counts if communication channel lost a reply, so some missed
   * replys can be ignored. */
  uint32_t ignoreMissedRepliesCount = 0;

  /** Pointer to the used FDIR instance. If not provided by child,
   * default class is instantiated. */
  FailureIsolationBase *fdirInstance;

  //! To correctly delete the default instance.
  bool defaultFDIRUsed;

  //! Indicates if SWITCH_WENT_OFF was already thrown.
  bool switchOffWasReported;

  /** Pointer to the task which executes this component,
      is invalid before setTaskIF was called. */
  PeriodicTaskIF *executingTask = nullptr;

  //! Object which switches power on and off.
  static object_id_t powerSwitcherId;

  //! Object which receives RAW data by default.
  static object_id_t rawDataReceiverId;

  //! Object which may be the root cause of an identified fault.
  static object_id_t defaultFdirParentId;

  /**
   * @brief   Send a reply to a received device handler command.
   *
   * This also resets #DeviceHandlerCommand to 0.
   *
   * @param reply the reply type
   * @param parameter parameter for the reply
   */
  void replyReturnvalueToCommand(ReturnValue_t status, uint32_t parameter = 0);
  /**
   * TODO: Whats the difference between this and the upper command?
   * @param status
   * @param parameter
   */
  void replyToCommand(ReturnValue_t status, uint32_t parameter = 0);

  /**
   * Do the transition to the main modes (MODE_ON, MODE_NORMAL and MODE_RAW).
   *
   * If the transition is complete, the mode should be set to the target mode,
   * which can be deduced from the current mode which is
   * [_MODE_TO_ON, _MODE_TO_NORMAL, _MODE_TO_RAW]
   *
   * The intended target submode is already set.
   * The origin submode can be read in subModeFrom.
   *
   * If the transition can not be completed, the child class can try to reach
   * an working mode by setting the mode either directly
   * or setting the mode to an transitional mode (TO_ON, TO_NORMAL, TO_RAW)
   * if the device needs to be reconfigured.
   *
   * If nothing works, the child class can wait for the timeout and the base
   * class will reset the mode to the mode where the transition
   * originated from (the child should report the reason for the failed transition).
   *
   * The intended way to send commands is to set a flag (enum) indicating
   * which command is to be sent here and then to check in
   * buildTransitionCommand() for the flag. This flag can also be used by
   * doStartUp() and doShutDown() to get a nice and clean implementation of
   * buildTransitionCommand() without switching through modes.
   *
   * When the the condition for the completion of the transition is met, the
   * mode can be set, for example in the scanForReply() function.
   *
   * The default implementation goes into the target mode directly.
   *
   * #transitionFailure can be set to a failure code indicating the reason
   * for a failed transition
   *
   * @param modeFrom
   * The mode the transition originated from:
   * [MODE_ON, MODE_NORMAL, MODE_RAW and _MODE_POWER_DOWN (if the mode changed
   * from _MODE_START_UP to _MODE_TO_ON)]
   * @param subModeFrom the subMode of modeFrom
   */
  virtual void doTransition(Mode_t modeFrom, Submode_t subModeFrom);

  /**
   * Get the communication action for the current step.
   * The step number can be read from #pstStep.
   * @return The communication action to execute in this step
   */
  virtual CommunicationAction getComAction();

  /**
   * Checks state of switches in conjunction with mode and triggers an event
   * if they don't fit.
   */
  virtual void checkSwitchState();

  /**
   * Reserved for the rare case where a device needs to perform additional
   * operation cyclically in OFF mode.
   */
  virtual void doOffActivity();

  /**
   * Reserved for the rare case where a device needs to perform additional
   * operation cyclically in ON mode.
   */
  virtual void doOnActivity();

  /**
   * Required for HasLocalDataPoolIF, return a handle to the local pool manager.
   * @return
   */
  LocalDataPoolManager *getHkManagerHandle() override;

  /**
   * Returns the delay cycle count of a reply.
   * A count != 0 indicates that the command is already executed.
   * @param deviceCommand	The command to look for
   * @return
   * The current delay count. If the command does not exist  (should never
   * happen) it returns 0.
   */
  uint8_t getReplyDelayCycles(DeviceCommandId_t deviceCommand);

  /**
   * Calls replyRawData() with #defaultRawReceiver, but checks if wiretapping
   * is active and if so, does not send the data as the wiretapping will have
   * sent it already
   */
  void replyRawReplyIfnotWiretapped(const uint8_t *data, size_t len);

  /**
   * Enable the reply checking for a command
   *
   * Is only called, if the command was sent (i.e. the getWriteReply was
   * successful). Must ensure that all replies are activated and correctly
   * linked to the command that initiated it.
   * The default implementation looks for a reply with the same id as the
   * command id in the replyMap or uses the alternativeReplyId if flagged so.
   * When found, copies maxDelayCycles to delayCycles in the reply information
   * and sets the command to expect one reply.
   *
   * Can be overwritten by the child, if a command activates multiple replies
   * or replyId differs from commandId.
   * Notes for child implementations:
   * 	- If the command was not found in the reply map,
   * 	  NO_REPLY_EXPECTED MUST be returned.
   * 	- A failure code may be returned if something went fundamentally wrong.
   *
   * @param deviceCommand
   * @return 	- returnvalue::OK if a reply was activated.
   * 			- NO_REPLY_EXPECTED if there was no reply found. This is not an
   * 			  error case as many commands do not expect a reply.
   */
  virtual ReturnValue_t enableReplyInReplyMap(DeviceCommandMap::iterator command,
                                              uint8_t expectedReplies = 1,
                                              bool useAlternateId = false,
                                              DeviceCommandId_t alternateReplyID = 0);

  /**
   * @brief   Build the device command to send for raw mode.
   * @details
   * This is only called in @c MODE_RAW. It is for the rare case that in
   * raw mode packets are to be sent by the handler itself. It is NOT needed
   * for the raw commanding service. Its only current use is in the STR
   * handler which gets its raw packets from a different source.
   * Also it can be used for transitional commands, to get the device ready
   * for @c MODE_RAW
   *
   * As it is almost never used, there is a default implementation
   * returning @c NOTHING_TO_SEND.
   *
   * #rawPacket and #rawPacketLen must be set by this method to the packet
   * to be sent.
   *
   * @param[out] id the device command id built
   * @return
   *    - @c returnvalue::OK when a command is to be sent
   *    - not @c NOTHING_TO_SEND when no command is to be sent
   */
  virtual ReturnValue_t buildChildRawCommand();

  /**
   * @brief   Construct a command reply containing a raw reply.
   * @details
   * It gets space in the #IPCStore, copies data there, then sends a raw reply
   * containing the store address. This method is virtual, as devices can have different channels
   * to send raw replies
   *
   * @param data data to send
   * @param len length of @c data
   * @param sendTo the messageQueueId of the one to send to
   * @param isCommand marks the raw data as a command, the message then
   * will be of type raw_command
   */
  virtual void replyRawData(const uint8_t *data, size_t len, MessageQueueId_t sendTo,
                            bool isCommand = false);

  /**
   * Get the state of the PCDU switches in the local datapool
   * @return
   *  - @c PowerSwitchIF::SWITCH_ON if all switches specified
   *       by #switches are on
   *  - @c PowerSwitchIF::SWITCH_OFF one of the switches specified by
   *       #switches are off
   *  - @c PowerSwitchIF::returnvalue::FAILED if an error occured
   */
  ReturnValue_t getStateOfSwitches();

  /**
   * Children can overwrite this function to suppress checking of the
   * command Queue
   *
   * This can be used when the child does not want to receive a command in
   * a certain situation. Care must be taken that checking is not
   * permanentely disabled as this would render the handler unusable.
   *
   * @return whether checking the queue should NOT be done
   */
  virtual bool dontCheckQueue();

  Mode_t getBaseMode(Mode_t transitionMode);

  bool isAwaitingReply();

  /**
   * Wrapper function for @handleDeviceTm which wraps the raw buffer with @SerialBufferAdapter.
   * For interpreted data, prefer the other function.
   * @param rawData
   * @param rawDataLen
   * @param replyId
   * @param forceDirectTm
   */
  void handleDeviceTm(const uint8_t *rawData, size_t rawDataLen, DeviceCommandId_t replyId,
                      bool forceDirectTm = false);
  /**
   * Can be used to handle Service 8 data replies. This will also generate the TM wiretapping
   * packets accordingly.
   * @param dataSet
   * @param replyId
   * @param forceDirectTm
   */
  void handleDeviceTm(const SerializeIF &dataSet, DeviceCommandId_t replyId,
                      bool forceDirectTm = false);

  virtual ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
                                         uint32_t *msToReachTheMode);

  virtual ReturnValue_t letChildHandleMessage(CommandMessage *message);

  /**
   * Overwrites SystemObject::triggerEvent in order to inform FDIR"Helper"
   * faster about executed events.
   * This is a bit sneaky, but improves responsiveness of the device FDIR.
   * @param event	The event to be thrown
   * @param parameter1	Optional parameter 1
   * @param parameter2	Optional parameter 2
   */
  void triggerEvent(Event event, uint32_t parameter1 = 0, uint32_t parameter2 = 0) override;
  /**
   * Same as triggerEvent, but for forwarding if object is used as proxy.
   */
  virtual void forwardEvent(Event event, uint32_t parameter1 = 0,
                            uint32_t parameter2 = 0) const override;

  /**
   * Checks if current mode is transitional mode.
   * @return true if mode is transitional, false else.
   */
  bool isTransitionalMode();

  /**
   * Checks if current handler state allows reception of external device commands.
   * Default implementation allows commands only in plain MODE_ON and MODE_NORMAL.
   * @return returnvalue::OK if commands are accepted, anything else otherwise.
   */
  virtual ReturnValue_t acceptExternalDeviceCommands();

  bool commandIsExecuting(DeviceCommandId_t commandId);

  /**
   * set all switches returned by getSwitches()
   *
   * @param onOff on == @c SWITCH_ON; off != @c SWITCH_ON
   */
  void commandSwitch(ReturnValue_t onOff);

  /**
   * @brief   This function can be used to insert device specific code during the do-send-read
   *          step.
   */
  virtual ReturnValue_t doSendReadHook();

 private:
  /**
   * State a cookie is in.
   *
   * Used to keep track of the state of the RMAP communication.
   */
  enum CookieState_t {
    COOKIE_UNUSED,       //!< The Cookie is unused
    COOKIE_WRITE_READY,  //!< There's data available to send.
    COOKIE_READ_SENT,    //!< A sendRead command was sent with this cookie
    COOKIE_WRITE_SENT    //!< A sendWrite command was sent with this cookie
  };
  /**
   * Information about a cookie.
   *
   * This is stored in a map for each cookie, to not only track the state,
   * but also information about the sent command. Tracking this information
   * is needed as the state of a commandId (waiting for reply) is done when a
   * write reply is received.
   */
  struct CookieInfo {
    CookieState_t state;
    DeviceCommandMap::iterator pendingCommand;
  };

  /**
   * @brief   Info about the #cookie
   * Used to track the state of the communication
   */
  CookieInfo cookieInfo;

  /** the object used to set power switches */
  PowerSwitchIF *powerSwitcher = nullptr;

  /** HK destination can also be set individually */
  object_id_t hkDestination = objects::NO_OBJECT;

  /**
   * @brief   Used for timing out mode transitions.
   * Set when setMode() is called.
   */
  uint32_t timeoutStart = 0;

  bool setStartupImmediately = false;

  /**
   * Delay for the current mode transition, used for time out
   */
  uint32_t childTransitionDelay;

  /**
   * @brief   The mode the current transition originated from
   *
   * This is private so the child can not change it and mess up the timeouts
   *
   * IMPORTANT: This is not valid during _MODE_SHUT_DOWN and _MODE_START_UP!!
   * (it is _MODE_POWER_DOWN during this modes)
   *
   * is element of [MODE_ON, MODE_NORMAL, MODE_RAW]
   */
  Mode_t transitionSourceMode;

  /**
   * the submode of the source mode during a transition
   */
  Submode_t transitionSourceSubMode;

  /**
   * read the command queue
   */
  void readCommandQueue(void);

  /**
   * Handle the device handler mode.
   *
   * - checks whether commands are valid for the current mode, rejects
   *   them accordingly
   * - checks whether commanded mode transitions are required and calls
   *   handleCommandedModeTransition()
   * - does the necessary action for the current mode or calls
   *   doChildStateMachine in modes @c MODE_TO_ON and @c MODE_TO_OFF
   * - actions that happen in transitions (e.g. setting a timeout) are
   *   handled in setMode()
   */
  void doStateMachine(void);

  void buildRawDeviceCommand(CommandMessage *message);
  void buildInternalCommand(void);
  /**
   * Decrement the counter for the timout of replies.
   *
   * This is called at the beginning of each cycle. It checks whether a
   * reply has timed out (that means a reply was expected but not received).
   */
  void decrementDeviceReplyMap(void);
  /**
   * Convenience function to handle a reply.
   *
   * Called after scanForReply() has found a packet. Checks if the found ID
   * is in the #deviceCommandMap, if so,  calls
   * #interpretDeviceReply for further action.
   *
   * It also resets the timeout counter for the command id.
   *
   * @param data the found packet
   * @param id the found id
   * @foundLen the length of the packet
   */
  void handleReply(const uint8_t *data, DeviceCommandId_t id, uint32_t foundLen);
  void replyToReply(const DeviceCommandId_t command, DeviceReplyInfo &replyInfo,
                    ReturnValue_t status);

  /**
   * Build and send a command to the device.
   *
   * This routine checks whether a raw or direct command has been received,
   * checks the content of the received command and calls
   * buildCommandFromCommand() for direct commands or sets #rawpacket
   * to the received raw packet.
   * If no external command is received or the received command is invalid and
   * the current mode is @c MODE_NORMAL or a transitional mode, it asks the
   * child class to build a command (via getNormalDeviceCommand() or
   * getTransitionalDeviceCommand() and buildCommand()) and
   * sends the command via RMAP.
   */
  void doSendWrite(void);
  /**
   * Check if the RMAP sendWrite action was successful.
   *
   * Depending on the result, the following is done
   * - if the device command was external commanded, a reply is sent
   *   indicating the result
   * - if the action was successful, the reply timout counter is initialized
   */
  void doGetWrite(void);
  /**
   * Send a RMAP getRead command.
   *
   * The size of the getRead command is #maxDeviceReplyLen.
   * This is always executed, independently from the current mode.
   */
  void doSendRead(void);
  /**
   * Check the getRead reply and the contained data.
   *
   * If data was received scanForReply() and, if successful, handleReply()
   * are called. If the current mode is @c MODE_RAW, the received packet
   * is sent to the commanding object via commandQueue.
   */
  void doGetRead(void);

  /**
   * @brief Resets replies which use a timeout to detect missed replies.
   */
  void resetTimeoutControlledReply(DeviceReplyInfo *info);

  /**
   * @brief Resets replies which use a number of maximum delay cycles to detect
   *        missed replies.
   */
  void resetDelayCyclesControlledReply(DeviceReplyInfo *info);

  /**
   * Retrive data from the #IPCStore.
   *
   * @param storageAddress
   * @param[out] data
   * @param[out] len
   * @return
   *   - @c returnvalue::OK @c data is valid
   *   - @c returnvalue::FAILED IPCStore is nullptr
   *   - the return value from the IPCStore if it was not @c returnvalue::OK
   */
  ReturnValue_t getStorageData(store_address_t storageAddress, uint8_t **data, size_t *len);

  /**
   * @param modeTo either @c MODE_ON, MODE_NORMAL or MODE_RAW, nothing else!
   */
  void setTransition(Mode_t modeTo, Submode_t submodeTo);

  /**
   * Calls the right child function for the transitional submodes
   */
  void callChildStatemachine();

  ReturnValue_t handleDeviceHandlerMessage(CommandMessage *message);

  virtual dur_millis_t getPeriodicOperationFrequency() const override;

  void parseReply(const uint8_t *receivedData, size_t receivedDataLen);

  void handleTransitionToOnMode(Mode_t commandedMode, Submode_t commandedSubmode);

  /**
   * Generic internal printer function which also handles printing the object ID.
   * @param errorType
   * @param functionName
   * @param errorCode
   * @param errorPrint
   */
  void printWarningOrError(sif::OutputTypes errorType, const char *functionName,
                           ReturnValue_t errorCode = returnvalue::FAILED,
                           const char *errorPrint = nullptr);
};

#endif /* FSFW_DEVICEHANDLERS_DEVICEHANDLERBASE_H_ */
