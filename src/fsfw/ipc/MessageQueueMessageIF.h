#ifndef FRAMEWORK_IPC_MESSAGEQUEUEMESSAGEIF_H_
#define FRAMEWORK_IPC_MESSAGEQUEUEMESSAGEIF_H_

#include <cstddef>
#include <cstdint>

#include "messageQueueDefinitions.h"

class MessageQueueMessageIF {
 public:
  /**
   * @brief   This constants defines the size of the header,
   *          which is added to every message.
   */
  static const size_t HEADER_SIZE = sizeof(MessageQueueId_t);

  virtual ~MessageQueueMessageIF() = default;

  /**
   * @brief	With this method, the whole content and the message
   * 			size is set to zero.
   * @details
   * Implementations should also take care to clear data which is stored
   * indirectly (e.g. storage data).
   */
  virtual void clear() = 0;

  /**
   * @brief	Get read-only pointer to the complete data of the message.
   * @return
   */
  [[nodiscard]] virtual const uint8_t* getBuffer() const = 0;

  /**
   * @brief	This method is used to get the complete data of the message.
   */
  virtual uint8_t* getBuffer() = 0;

  /**
   * @brief	This method is used to set the sender's message queue id
   * 			information prior to sending the message.
   * @param setId
   * The message queue id that identifies the sending message queue.
   */
  virtual void setSender(MessageQueueId_t setId) = 0;

  /**
   * @brief	This method is used to extract the sender's message queue id
   * 			information from a received message.
   */
  [[nodiscard]] virtual MessageQueueId_t getSender() const = 0;

  /**
   * @brief	This method is used to fetch the data content of the message.
   * @details
   * It shall be used by child classes to add data at the right position.
   */
  [[nodiscard]] virtual const uint8_t* getData() const = 0;
  /**
   * @brief	This method is used to fetch the data content of the message.
   * @details
   * It shall be used by child classes to add data at the right position.
   */
  virtual uint8_t* getData() = 0;

  /**
   * Get constant message size of current message implementation.
   * @return
   */
  [[nodiscard]] virtual size_t getMessageSize() const = 0;

  /**
   * Sets the current message size of a given message
   * @param messageSize
   */
  virtual void setMessageSize(size_t messageSize) = 0;
  /**
   * Returns the smallest possible message size, including any headers
   * @return
   */
  [[nodiscard]] virtual size_t getMinimumMessageSize() const = 0;
  /**
   * Returns the largest possible message size, including any headers
   * @return
   */
  [[nodiscard]] virtual size_t getMaximumMessageSize() const = 0;
  /**
   * Returns the largest possible data size without any headers
   * @return
   */
  [[nodiscard]] virtual size_t getMaximumDataSize() const = 0;
};

#endif /* FRAMEWORK_IPC_MESSAGEQUEUEMESSAGEIF_H_ */
