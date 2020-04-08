/**
 * @file	ModeMessage.h
 * @brief	This file defines the ModeMessage class.
 * @date	17.07.2013
 * @author	baetz
 */

#ifndef MODEMESSAGE_H_
#define MODEMESSAGE_H_

#include <framework/ipc/CommandMessage.h>

typedef uint32_t Mode_t;
typedef uint8_t Submode_t;

class ModeMessage {
private:
	ModeMessage();
public:
	static const uint8_t MESSAGE_ID = MESSAGE_TYPE::MODE_COMMAND;
	static const Command_t CMD_MODE_COMMAND = MAKE_COMMAND_ID(0x01);//!> Command to set the specified Mode, replies are: REPLY_MODE_REPLY, REPLY_WRONG_MODE_REPLY, and REPLY_REJECTED; don't add any replies, as this will break the subsystem mode machine!!
	static const Command_t CMD_MODE_COMMAND_FORCED = MAKE_COMMAND_ID(0xF1);//!> Command to set the specified Mode, regardless of external control flag, replies are: REPLY_MODE_REPLY, REPLY_WRONG_MODE_REPLY, and REPLY_REJECTED; don't add any replies, as this will break the subsystem mode machine!!
	static const Command_t REPLY_MODE_REPLY = MAKE_COMMAND_ID(0x02);//!> Reply to a CMD_MODE_COMMAND or CMD_MODE_READ
	static const Command_t REPLY_MODE_INFO = MAKE_COMMAND_ID(0x03);	//!> Unrequested info about the current mode (used for composites to inform their container of a changed mode)
	static const Command_t REPLY_CANT_REACH_MODE = MAKE_COMMAND_ID(0x04); //!> Reply in case a mode command can't be executed. Par1: returnCode, Par2: 0
	//SHOULDDO is there a way we can transmit a returnvalue when responding that the mode is wrong, so we can give a nice failure code when commanded by PUS?
	// shouldn't that possible with parameter 2 when submode only takes 1 byte?
	static const Command_t REPLY_WRONG_MODE_REPLY = MAKE_COMMAND_ID(0x05);//!> Reply to a CMD_MODE_COMMAND, indicating that a mode was commanded and a transition started but was aborted; the parameters contain the mode that was reached
	static const Command_t CMD_MODE_READ = MAKE_COMMAND_ID(0x06);//!> Command to read the current mode and reply with a REPLY_MODE_REPLY
	static const Command_t CMD_MODE_ANNOUNCE = MAKE_COMMAND_ID(0x07);//!> Command to trigger an ModeInfo Event. This command does NOT have a reply.
	static const Command_t CMD_MODE_ANNOUNCE_RECURSIVELY = MAKE_COMMAND_ID(0x08);//!> Command to trigger an ModeInfo Event and to send this command to every child. This command does NOT have a reply.

	static Mode_t getMode(const CommandMessage* message);
	static Submode_t getSubmode(const CommandMessage* message);
	static ReturnValue_t setModeMessage(CommandMessage* message,
			Command_t command, Mode_t mode, Submode_t submode);
	static void cantReachMode(CommandMessage* message, ReturnValue_t reason);
	static ReturnValue_t getCantReachModeReason(const CommandMessage* message);
	static void clear(CommandMessage* message);
};

#endif /* MODEMESSAGE_H_ */
