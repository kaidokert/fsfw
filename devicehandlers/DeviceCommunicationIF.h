#ifndef DEVICECOMMUNICATIONIF_H_
#define DEVICECOMMUNICATIONIF_H_

#include <framework/devicehandlers/Cookie.h>
#include <framework/devicehandlers/DeviceHandlerIF.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
/**
 * @defgroup interfaces Interfaces
 * @brief Interfaces for flight software objects
 */

/**
 * @defgroup communication comm
 * @brief Communication software components.
 */

/**
 * @brief This is an interface to decouple device communication from
 * the device handler to allow reuse of these components.
 * @details
 * Documentation: Dissertation Baetz p.138
 * It works with the assumption that received data
 * is polled by a component. There are four generic steps of device communication:
 *
 *   1. Send data to a device
 *   2. Get acknowledgement for sending
 *   3. Request reading data from a device
 *   4. Read received data
 *
 *  To identify different connection over a single interface can return
 *  so-called cookies to components.
 *  The CommunicationMessage message type can be used to extend the functionality of the
 *  ComIF if a separate polling task is required.
 *  @ingroup interfaces
 *  @ingroup comm
 */
class DeviceCommunicationIF: public HasReturnvaluesIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::DEVICE_COMMUNICATION_IF;

	static const ReturnValue_t INVALID_COOKIE_TYPE = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t NOT_ACTIVE = MAKE_RETURN_CODE(0x02);
	static const ReturnValue_t TOO_MUCH_DATA = MAKE_RETURN_CODE(0x03);
	static const ReturnValue_t NULLPOINTER = MAKE_RETURN_CODE(0x04);
	static const ReturnValue_t PROTOCOL_ERROR = MAKE_RETURN_CODE(0x05);

	virtual ~DeviceCommunicationIF() {}

	/**
	 * Called by DHB in the SEND_WRITE doSendWrite().
	 * This function is used to send data to the physical device
	 * by implementing and calling related drivers or wrapper functions.
	 * @param cookie
	 * @param data
	 * @param len
	 * @return -@c RETURN_OK for successfull send
	 *         - Everything else triggers sending failed event with
	 *           returnvalue as parameter 1
	 */
	virtual ReturnValue_t sendMessage(Cookie *cookie, const uint8_t * sendData,
			size_t sendLen) = 0;

	/**
	 * Called by DHB in the GET_WRITE doGetWrite().
	 * Get send confirmation that the data in sendMessage() was sent successfully.
	 * @param cookie
	 * @return -@c RETURN_OK if data was sent successfull
	 * 		   - Everything else triggers sending failed event with
	 *           returnvalue as parameter 1
	 */
	virtual ReturnValue_t getSendSuccess(Cookie *cookie) = 0;

	/**
	 * Called by DHB in the SEND_WRITE doSendRead().
	 * Request a reply.
	 * @param cookie
	 * @return
	 */
	virtual ReturnValue_t requestReceiveMessage(Cookie *cookie, size_t requestLen) = 0;

	/**
	 * Called by DHB in the GET_WRITE doGetRead().
	 * This function is used to receive data from the physical device
	 * by implementing and calling related drivers or wrapper functions.
	 * @param cookie
	 * @param data
	 * @param len
	 * @return @c RETURN_OK for successfull receive
	 *         - Everything else triggers receiving failed with
	 *           returnvalue as parameter 1
	 */
	virtual ReturnValue_t readReceivedMessage(Cookie *cookie, uint8_t **buffer,
			size_t *size) = 0;
};

#endif /* DEVICECOMMUNICATIONIF_H_ */
