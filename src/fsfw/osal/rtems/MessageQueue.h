#ifndef FSFW_OSAL_RTEMS_MESSAGEQUEUE_H_
#define FSFW_OSAL_RTEMS_MESSAGEQUEUE_H_

#include <fsfw/ipc/MessageQueueBase.h>

#include "RtemsBasic.h"
#include "fsfw/internalerror/InternalErrorReporterIF.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/MessageQueueMessage.h"
#include "fsfw/ipc/definitions.h"

/**
 *	@brief		This class manages sending and receiving of message queue messages.
 *
 *	@details	Message queues are used to pass asynchronous messages between processes.
 *				They work like post boxes, where all incoming messages are stored in
 *FIFO order. This class creates a new receiving queue and provides methods to fetch received
 *messages. Being a child of MessageQueueSender, this class also provides methods to send a message
 *to a user-defined or a default destination. In addition it also provides a reply method to answer
 *to the queue it received its last message from. The MessageQueue should be used as "post box" for
 *a single owning object. So all message queue communication is "n-to-one". For creating the queue,
 *as well as sending and receiving messages, the class makes use of the operating system calls
 *provided. \ingroup message_queue
 */
class MessageQueue : public MessageQueueBase {
 public:
  /**
   * @brief	The constructor initializes and configures the message queue.
   * @details	By making use of the according operating system call, a message queue is created
   * 			and initialized. The message depth - the maximum number of messages to be
   * 			buffered - may be set with the help of a parameter, whereas the message size
   * is automatically set to the maximum message queue message size. The operating system sets the
   * message queue id, or i case of failure, it is set to zero.
   * @param message_depth	The number of messages to be buffered before passing an error to the
   * 						sender. Default is three.
   * @param max_message_size	With this parameter, the maximum message size can be adjusted.
   * 							This should be left default.
   */
  explicit MessageQueue(size_t message_depth = 3,
                        size_t max_message_size = MessageQueueMessage::MAX_MESSAGE_SIZE,
                        MqArgs* args = nullptr);

  /** Copying message queues forbidden */
  MessageQueue(const MessageQueue&) = delete;
  MessageQueue& operator=(const MessageQueue&) = delete;

  /**
   * @brief	The destructor deletes the formerly created message queue.
   * @details	This is accomplished by using the delete call provided by the operating system.
   */
  ~MessageQueue() override;

  // Implement non-generic MessageQueueIF functions not handled by MessageQueueBase
  ReturnValue_t flush(uint32_t* count) override;

  ReturnValue_t receiveMessage(MessageQueueMessageIF* message) override;
  ReturnValue_t sendMessageFrom(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                MessageQueueId_t sentFrom, bool ignoreFault) override;

 private:
  /**
   * @brief	This attribute stores a reference to the internal error reporter for reporting full
   * queues. @details	In the event of a full destination queue, the reporter will be notified. The
   * reference is set by lazy loading
   */
  InternalErrorReporterIF* internalErrorReporter;

  static uint16_t queueCounter;
  /**
   * A method to convert an OS-specific return code to the frameworks return value concept.
   * @param inValue The return code coming from the OS.
   * @return The converted return value.
   */
  static ReturnValue_t convertReturnCode(rtems_status_code inValue);
};

#endif /* FSFW_OSAL_RTEMS_MESSAGEQUEUE_H_ */
