#ifndef FSFW_IPC_MESSAGEQUEUEIF_H_
#define FSFW_IPC_MESSAGEQUEUEIF_H_

#include <fsfw/ipc/definitions.h>

#include <cstdint>

#include "../returnvalues/returnvalue.h"
#include "MessageQueueMessageIF.h"
#include "messageQueueDefinitions.h"

// COULDDO: We could support blocking calls
// semaphores are being implemented, which makes this idea even more iteresting.

/**
 * @defgroup message_queue Message Queue
 * @brief Message Queue related software components
 */
class MessageQueueIF {
 public:
  static const MessageQueueId_t NO_QUEUE = 0;

  static const uint8_t INTERFACE_ID = CLASS_ID::MESSAGE_QUEUE_IF;
  //! No new messages on the queue
  static const ReturnValue_t EMPTY = MAKE_RETURN_CODE(1);
  //! [EXPORT] : [COMMENT] No space left for more messages
  static const ReturnValue_t FULL = MAKE_RETURN_CODE(2);
  //! [EXPORT] : [COMMENT] Returned if a reply method was called without partner
  static const ReturnValue_t NO_REPLY_PARTNER = MAKE_RETURN_CODE(3);
  //! [EXPORT] : [COMMENT] Returned if the target destination is invalid.
  static constexpr ReturnValue_t DESTINATION_INVALID = MAKE_RETURN_CODE(4);

  virtual ~MessageQueueIF() = default;
  /**
   * @brief	This operation sends a message to the last communication partner.
   * @details
   * This operation simplifies answering an incoming message by using the
   * stored lastParnter information as destination. If there was no message
   * received yet (i.e. lastPartner is zero), an error code is returned.
   * @param message
   * A pointer to a previously created message, which is sent.
   * @return
   * -@c returnvalue::OK if ok
   * -@c NO_REPLY_PARTNER Should return NO_REPLY_PARTNER if partner was found.
   */
  virtual ReturnValue_t reply(MessageQueueMessageIF* message) = 0;

  /**
   * @brief	This function reads available messages from the message queue and returns the
   * sender.
   * @details
   * It works identically to the other receiveMessage call, but in addition
   * returns the sender's queue id.
   * @param message
   * A pointer to a message in which the received data is stored.
   * @param receivedFrom
   * A pointer to a queue id in which the sender's id is stored.
   */
  virtual ReturnValue_t receiveMessage(MessageQueueMessageIF* message,
                                       MessageQueueId_t* receivedFrom) = 0;

  /**
   * @brief	This function reads available messages from the message queue.
   * @details
   * If data is available it is stored in the passed message pointer.
   * The message's original content is overwritten and the sendFrom
   * information is stored in theblastPartner attribute. Else, the lastPartner
   * information remains untouched, the message's content is cleared and the
   * function returns immediately.
   * @param message
   * A pointer to a message in which the received data is stored.
   * @return -@c returnvalue::OK on success
   *         -@c MessageQueueIF::EMPTY if queue is empty
   */
  virtual ReturnValue_t receiveMessage(MessageQueueMessageIF* message) = 0;
  /**
   * Deletes all pending messages in the queue.
   * @param count The number of flushed messages.
   * @return returnvalue::OK on success.
   */
  virtual ReturnValue_t flush(uint32_t* count) = 0;
  /**
   * @brief	This method returns the message queue ID of the last communication partner.
   */
  [[nodiscard]] virtual MessageQueueId_t getLastPartner() const = 0;
  /**
   * @brief	This method returns the message queue ID  of this class's message queue.
   */
  [[nodiscard]] virtual MessageQueueId_t getId() const = 0;

  /**
   * @brief	With the sendMessage call, a queue message is sent to a receiving queue.
   * @details
   * This method takes the message provided, adds the sentFrom information
   * and passes it on to the destination provided with an operating system
   * call. The OS's returnvalue is returned.
   * @param sendTo
   * This parameter specifies the message queue id to send the message to.
   * @param message
   * This is a pointer to a previously created message, which is sent.
   * @param sentFrom
   * The sentFrom information can be set to inject the sender's queue id
   * into the message. This variable is set to zero by default.
   * @param ignoreFault
   * If set to true, the internal software fault counter is not incremented
   * if queue is full (if implemented).
   * @return -@c returnvalue::OK on success
   *         -@c MessageQueueIF::FULL if queue is full
   */
  virtual ReturnValue_t sendMessageFrom(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                        MessageQueueId_t sentFrom, bool ignoreFault = false) = 0;

  /**
   * @brief	This operation sends a message to the given destination.
   * @details
   * It directly uses the sendMessage call of the MessageQueueSender parent,
   * but passes its queue id as "sentFrom" parameter.
   * @param sendTo
   * This parameter specifies the message queue id of the destination
   * message queue.
   * @param message
   * A pointer to a previously created message, which is sent.
   * @param ignoreFault
   * If set to true, the internal software fault counter is not incremented
   * if queue is full.
   */
  virtual ReturnValue_t sendMessage(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                    bool ignoreFault = false) = 0;

  /**
   * @brief	The sendToDefaultFrom method sends a queue message to the default destination.
   * @details
   * In all other aspects, it works identical to the sendMessage method.
   * @param message
   * This is a pointer to a previously created message, which is sent.
   * @param sentFrom
   * The sentFrom information can be set to inject the sender's queue id
   * into the message. This variable is set to zero by default.
   * @return -@c returnvalue::OK on success
   *         -@c MessageQueueIF::FULL if queue is full
   */
  virtual ReturnValue_t sendToDefaultFrom(MessageQueueMessageIF* message, MessageQueueId_t sentFrom,
                                          bool ignoreFault = false) = 0;
  /**
   * @brief	This operation sends a message to the default destination.
   * @details
   * As in the sendMessage method, this function uses the sendToDefault
   * call of the Implementation class and adds its queue id as
   * "sentFrom" information.
   * @param message	A pointer to a previously created message, which is sent.
   * @return -@c returnvalue::OK on success
   *         -@c MessageQueueIF::FULL if queue is full
   */
  virtual ReturnValue_t sendToDefault(MessageQueueMessageIF* message) = 0;
  /**
   * @brief	This method is a simple setter for the default destination.
   */
  virtual void setDefaultDestination(MessageQueueId_t defaultDestination) = 0;
  /**
   * @brief	This method is a simple getter for the default destination.
   */
  [[nodiscard]] virtual MessageQueueId_t getDefaultDestination() const = 0;

  [[nodiscard]] virtual bool isDefaultDestinationSet() const = 0;

  virtual MqArgs& getMqArgs() = 0;
};

#endif /* FSFW_IPC_MESSAGEQUEUEIF_H_ */
