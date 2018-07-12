#ifndef MESSAGEQUEUEMESSAGE_H_
#define MESSAGEQUEUEMESSAGE_H_

#include <framework/ipc/MessageQueueSenderIF.h>
#include <stddef.h>

/**
 *	\brief		This class is the representation and data organizer for interprocess messages.
 *
 *	\details	To facilitate and standardize interprocess communication, this class was created
 *				to handle a lightweight "interprocess message protocol". It adds a header with the
 *				sender's queue id to every sent message and defines the maximum total message size.
 *				Specialized messages, such as device commanding messages, can be created by inheriting
 *				from this class and filling the buffer provided by getData with additional content.
 *				If larger amounts of data must be sent between processes, the data shall be stored in
 *				the IPC Store object and only the storage id is passed in a queue message.
 *				The class is used both to generate and send messages and to receive messages from
 *				other tasks.
 *	\ingroup message_queue
 */
class MessageQueueMessage {
public:
	/**
	 * \brief	This constant defines the maximum size of the data content, excluding the header.
	 * \details	It may be changed if necessary, but in general should be kept as small as possible.
	 */
	static const size_t MAX_DATA_SIZE = 24;
	/**
	 * \brief	This constants defines the size of the header, which is added to every message.
	 */
	static const size_t HEADER_SIZE = sizeof(MessageQueueId_t);
	/**
	 * \brief	This constant defines the maximum total size in bytes of a sent message.
	 * \details	It is the sum of the maximum data and the header size. Be aware that this constant
	 * 			is used to define the buffer sizes for every message queue in the system. So, a change
	 * 			here may have significant impact on the required resources.
	 */
	static const size_t MAX_MESSAGE_SIZE = MAX_DATA_SIZE + HEADER_SIZE;
private:
	/**
	 * \brief	This is the internal buffer that contains the actual message data.
	 */
	uint8_t internalBuffer[MAX_MESSAGE_SIZE];
public:
	/**
	 * \brief	The size information of each message is stored in this attribute.
	 * \details	It is public to simplify usage and to allow for passing the variable's address as a
	 * 			pointer. Care must be taken when inheriting from this class, as every child class is
	 * 			responsible for managing the size information by itself. When using the class to
	 * 			receive a message, the size information is updated automatically.
	 */
	size_t messageSize;
	/**
	 * \brief	The class is initialized empty with this constructor.
	 * \details	The messageSize attribute is set to the header's size and the whole content is set to
	 * 			zero.
	 */
	MessageQueueMessage();
	/**
	 * \brief	With this constructor the class is initialized with the given content.
	 * \details	If the passed message size fits into the buffer, the passed data is copied to the
	 * 			internal buffer and the messageSize information is set. Otherwise, messageSize
	 * 			is set to the header's size and the whole content is set to zero.
	 * \param data	The data to be put in the message.
	 * \param size	Size of the data to be copied. Must be smaller than MAX_MESSAGE_SIZE.
	 */
	MessageQueueMessage(uint8_t* data, uint32_t size);
	/**
	 * \brief	As no memory is allocated in this class, the destructor is empty.
	 */
	virtual ~MessageQueueMessage();
	/**
	 * \brief	This method is used to get the complete data of the message.
	 */
	const uint8_t* getBuffer() const;
	/**
	 * \brief	This method is used to get the complete data of the message.
	 */
	uint8_t* getBuffer();
	/**
	 * \brief	This method is used to fetch the data content of the message.
	 * \details It shall be used by child classes to add data at the right position.
	 */
	const uint8_t* getData() const;
	/**
	 * \brief	This method is used to fetch the data content of the message.
	 * \details It shall be used by child classes to add data at the right position.
	 */
	uint8_t* getData();
	/**
	 * \brief	This method is used to extract the sender's message queue id information from a
	 * 			received message.
	 */
	MessageQueueId_t getSender() const;
	/**
	 * \brief	With this method, the whole content and the message size is set to zero.
	 */
	void clear();
	/**
	 * \brief	This is a debug method that prints the content (till messageSize) to the debug output.
	 */
	void print();
	/**
	 * \brief	This method is used to set the sender's message queue id information prior to
	 * 			sending the message.
	 * \param setId	The message queue id that identifies the sending message queue.
	 */
	void setSender(MessageQueueId_t setId);
	/**
	 * \brief	This helper function is used by the MessageQueue class to check the size of an
	 * 			incoming message.
	 * \details	The method must be overwritten by child classes if size checks shall be more strict.
	 * @return	The default implementation returns HEADER_SIZE.
	 */
	virtual size_t getMinimumMessageSize();
};

#endif /* MESSAGEQUEUEMESSAGE_H_ */
