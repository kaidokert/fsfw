#ifndef ACCEPTSTELECOMMANDSIF_H_
#define ACCEPTSTELECOMMANDSIF_H_

#include "../ipc/MessageQueueSenderIF.h"

/**
 * @brief 	This interface is implemented by classes that are sinks for
 * 			Telecommands.
 * @details	Any service receiving telecommands shall implement this interface
 * 			and thus make the service id and the receiving message queue public.
 */
class AcceptsTelecommandsIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::ACCEPTS_TELECOMMANDS_IF;
	static const ReturnValue_t ACTIVITY_STARTED = MAKE_RETURN_CODE(1);
	static const ReturnValue_t INVALID_SUBSERVICE = MAKE_RETURN_CODE(2);
	static const ReturnValue_t ILLEGAL_APPLICATION_DATA = MAKE_RETURN_CODE(3);
	static const ReturnValue_t SEND_TM_FAILED = MAKE_RETURN_CODE(4);
	static const ReturnValue_t TIMEOUT = MAKE_RETURN_CODE(5);
	/**
	 * @brief The virtual destructor as it is mandatory for C++ interfaces.
	 */
	virtual ~AcceptsTelecommandsIF() {

	}
	/**
	 * @brief	Getter for the service id.
	 * @details	Any receiving service (at least any PUS service) shall have a
	 * 			service id. If the receiver can handle Telecommands, but for
	 * 			some reason has no service id, it shall return 0.
	 * @return	The service id or 0.
	 */
	virtual uint16_t getIdentifier() = 0;
	/**
	 * @brief	This method returns the message queue id of the telecommand
	 * 			receiving message queue.
	 * @return	The telecommand reception message queue id.
	 */
	virtual MessageQueueId_t getRequestQueue() = 0;
};


#endif /* ACCEPTSTELECOMMANDSIF_H_ */
