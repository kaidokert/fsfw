/**
 * @file	TimeMessage.h
 * @brief	This file defines the TimeMessage class.
 * @date	26.02.2013
 * @author	baetz
 */

#ifndef TIMEMESSAGE_H_
#define TIMEMESSAGE_H_

#include <framework/ipc/MessageQueueMessage.h>

class TimeMessage : public MessageQueueMessage {
protected:
	/**
	 * @brief	This call always returns the same fixed size of the message.
	 * @return	Returns HEADER_SIZE + \c sizeof(timeval).
	 */
	size_t getMinimumMessageSize();
public:

	/**
	 * @ brief the size of a TimeMessage
	 */
	static const uint32_t MAX_SIZE = HEADER_SIZE + sizeof(timeval);

	/**
	 * @brief	In the default constructor, only the message_size is set.
	 */
	TimeMessage();
	/**
	 * @brief	With this constructor, the passed time information is directly put
	 * 			into the message.
	 * @param setTime The time information to put into the message.
	 */
	TimeMessage( timeval setTime );
	/**
	 * @brief	The class's destructor is empty.
	 */
	~TimeMessage();
	/**
	 * @brief	This getter returns the time information in timeval format.
	 * @return	Returns the time stored in this packet.
	 */
	timeval getTime();
};

#endif /* TIMEMESSAGE_H_ */
