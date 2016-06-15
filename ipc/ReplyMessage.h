/**
 * @file	ReplyMessage.h
 * @brief	This file defines the ReplyMessage class.
 * @date	20.06.2013
 * @author	baetz
 */

#ifndef REPLYMESSAGE_H_
#define REPLYMESSAGE_H_

#include <framework/ipc/CommandMessage.h>

class ReplyMessage: public CommandMessage {
public:
	static const uint8_t MESSAGE_ID = 1;
	static const Command_t REPLY_MODE = MAKE_COMMAND_ID( 3 );	//!< Reply to a @c CMD_MODE or @c CMD_READ_MODE, getParameter contains a DeviceHandlerIF::DeviceHandlerMode_t, getParameter2 the submode
	static const Command_t REPLY_TRANSITION_DELAY = MAKE_COMMAND_ID( 4 );	//!< Reply to a @c CMD_MODE, indicates that the transition will take some time, getParameter contains the maximum duration in ms
	static const Command_t REPLY_INVALID_MODE = MAKE_COMMAND_ID( 5 );	//!< Reply to a @c CMD_MODE, indicates that the requested DeviceHandlerIF::DeviceHandlerMode_t was invalid
	static const Command_t REPLY_CANT_REACH_MODE = MAKE_COMMAND_ID( 6 );	//!< Reply to a @c CMD_MODE, indicates that the requested DeviceHandlerIF::DeviceHandlerMode_t can not be reached from the current mode, getParameter() is the DeviceHandlerIF::DeviceHandlerMode_t, getParameter2() is the submode number
	ReplyMessage() :
			CommandMessage() {
	}
	ReplyMessage(Command_t command, uint32_t parameter1, uint32_t parameter2) :
			CommandMessage(command, parameter1, parameter2) {
	}
	virtual ~ReplyMessage() {}
};

#endif /* REPLYMESSAGE_H_ */
