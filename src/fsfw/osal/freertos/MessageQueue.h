#ifndef FSFW_OSAL_FREERTOS_MESSAGEQUEUE_H_
#define FSFW_OSAL_FREERTOS_MESSAGEQUEUE_H_

#include <fsfw/ipc/MessageQueueBase.h>

#include "FreeRTOS.h"
#include "TaskManagement.h"
#include "fsfw/internalerror/InternalErrorReporterIF.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/MessageQueueMessage.h"
#include "fsfw/ipc/MessageQueueMessageIF.h"
#include "fsfw/ipc/definitions.h"
#include "queue.h"

/**
 * @brief		This class manages sending and receiving of
 * 				message queue messages.
 * @details
 * Message queues are used to pass asynchronous messages between processes.
 * They work like post boxes, where all incoming messages are stored in FIFO
 * order. This class creates a new receiving queue and provides methods to fetch
 * received messages. Being a child of MessageQueueSender, this class also
 * provides methods to send a message to a user-defined or a default destination.
 * In addition it also provides a reply method to answer to the queue it
 * received its last message from.
 *
 * The MessageQueue should be used as "post box" for a single owning object.
 * So all message queue communication is "n-to-one".
 * For creating the queue, as well as sending and receiving messages, the class
 * makes use of the operating system calls provided.
 *
 * Please keep in mind that FreeRTOS offers different calls for message queue
 * operations if called from an ISR.
 * For now, the system context needs to be switched manually.
 * @ingroup osal
 * @ingroup message_queue
 */
class MessageQueue : public MessageQueueBase {
  friend class MessageQueueSenderIF;

 public:
  /**
   * @brief	The constructor initializes and configures the message queue.
   * @details
   * By making use of the according operating system call, a message queue
   * is created and initialized. The message depth - the maximum number of
   * messages to be buffered - may be set with the help of a parameter,
   * whereas the message size is automatically set to the maximum message
   * queue message size. The operating system sets the message queue id, or
   * in case of failure, it is set to zero.
   * @param message_depth
   * The number of messages to be buffered before passing an error to the
   * sender. Default is three.
   * @param max_message_size
   * With this parameter, the maximum message size can be adjusted.
   * This should be left default.
   */
  MessageQueue(size_t messageDepth = 3,
               size_t maxMessageSize = MessageQueueMessage::MAX_MESSAGE_SIZE,
               MqArgs* args = nullptr);

  /** Copying message queues forbidden */
  MessageQueue(const MessageQueue&) = delete;
  MessageQueue& operator=(const MessageQueue&) = delete;

  /**
   * @brief	The destructor deletes the formerly created message queue.
   * @details	This is accomplished by using the delete call provided
   * 			by the operating system.
   */
  virtual ~MessageQueue();

  /**
   * This function is used to switch the call context. This has to be called
   * if a message is sent or received from an ISR!
   * @param callContext
   */
  void switchSystemContext(CallContext callContext);

  QueueHandle_t getNativeQueueHandle();

  // Implement non-generic MessageQueueIF functions not handled by MessageQueueBase
  virtual ReturnValue_t sendMessageFrom(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                        MessageQueueId_t sentFrom = NO_QUEUE,
                                        bool ignoreFault = false) override;
  ReturnValue_t receiveMessage(MessageQueueMessageIF* message) override;
  ReturnValue_t flush(uint32_t* count) override;

 protected:
  /**
   * @brief 	Implementation to be called from any send Call within
   * 			MessageQueue and MessageQueueSenderIF.
   * @details
   * This method takes the message provided, adds the sentFrom information and
   * passes it on to the destination provided with an operating system call.
   * The OS's return value is returned.
   * @param sendTo
   * This parameter specifies the message queue id to send the message to.
   * @param message
   * This is a pointer to a previously created message, which is sent.
   * @param sentFrom
   * The sentFrom information can be set to inject the sender's queue id into
   * the message. This variable is set to zero by default.
   * @param ignoreFault
   * If set to true, the internal software fault counter  is not incremented
   * if queue is full.
   * @param context Specify whether call is made from task or from an ISR.
   */
  static ReturnValue_t sendMessageFromMessageQueue(MessageQueueId_t sendTo,
                                                   MessageQueueMessageIF* message,
                                                   MessageQueueId_t sentFrom = NO_QUEUE,
                                                   bool ignoreFault = false,
                                                   CallContext callContext = CallContext::TASK);

  static ReturnValue_t handleSendResult(BaseType_t result, bool ignoreFault);

 private:
  QueueHandle_t handle;

  const size_t maxMessageSize;
  //! Stores the current system context
  CallContext callContext = CallContext::TASK;
};

#endif /* FSFW_OSAL_FREERTOS_MESSAGEQUEUE_H_ */
