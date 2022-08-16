#ifndef FSFW_ACTION_ACTIONHELPER_H_
#define FSFW_ACTION_ACTIONHELPER_H_

#include "ActionMessage.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/serialize/SerializeIF.h"
/**
 * @brief Action Helper is a helper class which handles action messages
 *
 * Components which use the HasActionIF this helper can be used to handle
 * the action messages.
 * It does handle step messages as well as other answers to action calls.
 * It uses the executeAction function of its owner as callback.
 * The call of the initialize function is mandatory and needs a
 * valid MessageQueueIF pointer!
 */
class HasActionsIF;

class ActionHelper {
 public:
  /**
   * Constructor of the action helper
   * @param setOwner Pointer to the owner of the interface
   * @param useThisQueue  messageQueue to be used, can be set during
   *                      initialize function as well.
   */
  ActionHelper(HasActionsIF* setOwner, MessageQueueIF* useThisQueue);

  virtual ~ActionHelper();
  /**
   * Function to be called from the owner with a new command message
   *
   * If the message is a valid action message the helper will use the
   * executeAction function from HasActionsIF.
   * If the message is invalid or the callback fails a message reply will be
   * send to the sender of the message automatically.
   *
   * @param command Pointer to a command message received by the owner
   * @return returnvalue::OK if the message is a action message,
   *         CommandMessage::UNKNOW_COMMAND if this message ID is unkown
   */
  ReturnValue_t handleActionMessage(CommandMessage* command);
  /**
   * Helper initialize function. Must be called before use of any other
   * helper function
   * @param queueToUse_   Pointer to the messageQueue to be used, optional
   *                      if queue was set in constructor
   * @return Returns returnvalue::OK if successful
   */
  ReturnValue_t initialize(MessageQueueIF* queueToUse_ = nullptr);
  /**
   * Function to be called from the owner to send a step message.
   * Success or failure will be determined by the result value.
   *
   * @param step Number of steps already done
   * @param reportTo The messageQueueId to report the step message to
   * @param commandId ID of the executed command
   * @param result Result of the execution
   */
  void step(uint8_t step, MessageQueueId_t reportTo, ActionId_t commandId,
            ReturnValue_t result = returnvalue::OK);
  /**
   * Function to be called by the owner to send a action completion message
   * @param success   Specify whether action was completed successfully or not.
   * @param reportTo MessageQueueId_t to report the action completion message to
   * @param commandId ID of the executed command
   * @param result Result of the execution
   */
  void finish(bool success, MessageQueueId_t reportTo, ActionId_t commandId,
              ReturnValue_t result = returnvalue::OK);
  /**
   * Function to be called by the owner if an action does report data.
   * Takes a SerializeIF* pointer and serializes it into the IPC store.
   * @param reportTo  MessageQueueId_t to report the action completion
   *                  message to
   * @param replyId ID of the executed command
   * @param data Pointer to the data
   * @return Returns returnvalue::OK if successful, otherwise failure code
   */
  ReturnValue_t reportData(MessageQueueId_t reportTo, ActionId_t replyId, SerializeIF* data,
                           bool hideSender = false);
  /**
   * Function to be called by the owner if an action does report data.
   * Takes the raw data and writes it into the IPC store.
   * @param reportTo  MessageQueueId_t to report the action completion
   *                  message to
   * @param replyId ID of the executed command
   * @param data Pointer to the data
   * @return Returns returnvalue::OK if successful, otherwise failure code
   */
  ReturnValue_t reportData(MessageQueueId_t reportTo, ActionId_t replyId, const uint8_t* data,
                           size_t dataSize, bool hideSender = false);
  /**
   * Function to setup the MessageQueueIF* of the helper. Can be used to
   * set the MessageQueueIF* if message queue is unavailable at construction
   * and initialize but must be setup before first call of other functions.
   * @param queue Queue to be used by the helper
   */
  void setQueueToUse(MessageQueueIF* queue);

 protected:
  //! Increase of value of this per step
  static const uint8_t STEP_OFFSET = 1;
  //! Pointer to the owner
  HasActionsIF* owner;
  //! Queue to be used as response sender, has to be set in ctor or with
  //! setQueueToUse
  MessageQueueIF* queueToUse;
  //! Pointer to an IPC Store, initialized during construction or
  StorageManagerIF* ipcStore = nullptr;

  /**
   * Internal function called by handleActionMessage
   * @param commandedBy MessageQueueID of Commander
   * @param actionId ID of action to be done
   * @param dataAddress Address of additional data in IPC Store
   */
  virtual void prepareExecution(MessageQueueId_t commandedBy, ActionId_t actionId,
                                store_address_t dataAddress);
  /**
   * @brief Default implementation is empty.
   */
  virtual void resetHelper();
};

#endif /* FSFW_ACTION_ACTIONHELPER_H_ */
