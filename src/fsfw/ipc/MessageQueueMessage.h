#ifndef FSFW_IPC_MESSAGEQUEUEMESSAGE_H_
#define FSFW_IPC_MESSAGEQUEUEMESSAGE_H_

#include <cstddef>

#include "fsfw/ipc/MessageQueueMessageIF.h"

/**
 *	@brief		This class is the representation and data organizer
 *				for interprocess messages.
 *	@details
 *	To facilitate and standardize interprocess communication, this class was
 *	created to handle a lightweight "interprocess message protocol".
 *
 *	It adds a header with the sender's queue id to every sent message and
 *	defines the maximum total message size. Specialized messages, such as
 *	device commanding messages, can be created by inheriting from this class
 *	and filling the buffer provided by getData with additional content.
 *
 *	If larger amounts of data must be sent between processes, the data shall
 *	be stored in the IPC Store object and only the storage id is passed in a
 *	queue message.The class is used both to generate and send messages and to
 *	receive messages from other tasks.
 *	@ingroup message_queue
 */
class MessageQueueMessage : public MessageQueueMessageIF {
 public:
  /**
   * @brief	This constant defines the maximum size of the data content,
   * 			excluding the header.
   * @details
   * It may be changed if necessary, but in general should be kept
   * as small as possible.
   */
  static const size_t MAX_DATA_SIZE = 24;
  /**
   * @brief	This constant defines the maximum total size in bytes
   * 			of a sent message.
   * @details
   * It is the sum of the maximum data and the header size. Be aware that
   * this constant is used to define the buffer sizes for every message
   * queue in the system. So, a change here may have significant impact on
   * the required resources.
   */
  static constexpr size_t MAX_MESSAGE_SIZE = MAX_DATA_SIZE + HEADER_SIZE;
  /**
   * @brief Defines the minimum size of a message where only the
   * 		  header is included
   */
  static constexpr size_t MIN_MESSAGE_SIZE = HEADER_SIZE;

  /**
   * @brief	The class is initialized empty with this constructor.
   * @details
   * The messageSize attribute is set to the header's size and the whole
   * content is set to zero.
   */
  MessageQueueMessage();
  /**
   * @brief	With this constructor the class is initialized with
   * 			the given content.
   * @details
   * If the passed message size fits into the buffer, the passed data is
   * copied to the internal buffer and the messageSize information is set.
   * Otherwise, messageSize is set to the header's size and the whole
   * content is set to zero.
   * @param data	The data to be put in the message.
   * @param size	Size of the data to be copied. Must be smaller than
   * 				MAX_MESSAGE_SIZE and larger than MIN_MESSAGE_SIZE.
   */
  MessageQueueMessage(uint8_t* data, size_t size);

  /**
   * @brief	As no memory is allocated in this class,
   * 		    the destructor is empty.
   */
  ~MessageQueueMessage() override;

  /**
   * @brief	This method is used to get the complete data of the message.
   */
  [[nodiscard]] const uint8_t* getBuffer() const override;
  /**
   * @brief	This method is used to get the complete data of the message.
   */
  uint8_t* getBuffer() override;
  /**
   * @brief	This method is used to fetch the data content of the message.
   * @details
   * It shall be used by child classes to add data at the right position.
   */
  [[nodiscard]] const uint8_t* getData() const override;
  /**
   * @brief	This method is used to fetch the data content of the message.
   * @details
   * It shall be used by child classes to add data at the right position.
   */
  uint8_t* getData() override;
  /**
   * @brief	This method is used to extract the sender's message
   * 			queue id information from a received message.
   */
  [[nodiscard]] MessageQueueId_t getSender() const override;
  /**
   * @brief	With this method, the whole content
   * 			and the message size is set to zero.
   */
  void clear() override;

  /**
   * @brief	This method is used to set the sender's message queue id
   * 			information prior to ing the message.
   * @param setId
   * The message queue id that identifies the sending message queue.
   */
  void setSender(MessageQueueId_t setId) override;

  [[nodiscard]] size_t getMessageSize() const override;
  void setMessageSize(size_t messageSize) override;
  [[nodiscard]] size_t getMinimumMessageSize() const override;
  [[nodiscard]] size_t getMaximumMessageSize() const override;
  [[nodiscard]] size_t getMaximumDataSize() const override;

  /**
   * @brief	This is a debug method that prints the content.
   */
  void print(bool printWholeMessage);

  /**
   * TODO: This really should not be public. If it should be possible to pass size address as a
   *       pointer, add a getter function returning a const reference to the size
   * @brief	The size information of each message is stored in
   *  			this attribute.
   * @details
   * It is public to simplify usage and to allow for passing the size
   * address as a pointer. Care must be taken when inheriting from this class,
   * as every child class is responsible for managing the size information by
   * itself. When using the class to receive a message, the size information
   * is updated automatically.
   *
   * Please note that the minimum size is limited by the size of the header
   * while the maximum size is limited by the maximum allowed message size.
   */
  size_t messageSize;

 private:
  /**
   * @brief	This is the internal buffer that contains the
   * 			actual message data.
   */
  uint8_t internalBuffer[MAX_MESSAGE_SIZE] = {};
};

#endif /* FSFW_IPC_MESSAGEQUEUEMESSAGE_H_ */
