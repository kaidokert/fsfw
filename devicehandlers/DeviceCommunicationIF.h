#ifndef DEVICECOMMUNICATIONIF_H_
#define DEVICECOMMUNICATIONIF_H_

#include <framework/devicehandlers/Cookie.h>
#include <framework/devicehandlers/DeviceHandlerIF.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
/**
 * @defgroup interfaces Interfaces
 * @brief Communication interfaces for flight software objects
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
 *  To identify different connection over a single interface can return so-called cookies to components.
 *  The CommunicationMessage message type can be used to extend the functionality of the
 *  ComIF if a separate polling task is required.
 */
class DeviceCommunicationIF: public HasReturnvaluesIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::DEVICE_COMMUNICATION_IF;

	static const ReturnValue_t INVALID_COOKIE_TYPE = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t NOT_ACTIVE = MAKE_RETURN_CODE(0x02);
	static const ReturnValue_t INVALID_ADDRESS = MAKE_RETURN_CODE(0x03);
	static const ReturnValue_t TOO_MUCH_DATA = MAKE_RETURN_CODE(0x04);
	static const ReturnValue_t NULLPOINTER = MAKE_RETURN_CODE(0x05);
	static const ReturnValue_t PROTOCOL_ERROR = MAKE_RETURN_CODE(0x06);
	static const ReturnValue_t CANT_CHANGE_REPLY_LEN = MAKE_RETURN_CODE(0x07);

	virtual ~DeviceCommunicationIF() {}

	/**
	 * Open a connection. Define a communication specific cookie which can
	 * be used to store information about the communication.
	 * The two optional parameter provide additional flexibility to
	 * set up the communication interface as desired. If there are a lot of
	 * variables to set, a store ID to the parameters stored in the IPC store
	 * can also be passed.
	 *
	 * @param cookie [out] This data class stores information about the communication.
	 * @param address Logical device address
	 * @param maxReplyLen Maximum length of expected reply
	 * @param comParameter1 Arbitrary parameter which can be used to set up the cookie or comIF.
	 * @param comParameter2 Arbitrary parameter which can be used to set up the cookie or comIF.
	 * @return
	 */
	virtual ReturnValue_t open(Cookie **cookie, address_t address,
			uint32_t maxReplyLen, uint32_t comParameter1, uint32_t comParameter2) = 0;

	/**
	 * Use an existing cookie to open a connection to a new DeviceCommunication.
	 * The previous connection must not be closed.
	 *
	 * @param cookie
	 * @param address
	 * @param maxReplyLen
	 * @return -@c RETURN_OK New communication set up successfully
	 *         - Everything else: Cookie is unchanged and can be used with
	 *           previous connection
	 */
	virtual ReturnValue_t reOpen(Cookie *cookie, address_t address,
			uint32_t maxReplyLen, uint32_t comParameter1, uint32_t comParameter2) = 0;

	/**
	 * Closing call of connection. Don't forget to free memory of cookie.
	 * @param cookie
	 */
	virtual void close(Cookie *cookie) = 0;

	/**
	 * Called by DHB in the SEND_WRITE doSendWrite().
	 * This function is used to send data to the physical device
	 * by implementing and calling related drivers or wrapper functions.
	 * @param cookie
	 * @param data
	 * @param len
	 * @return -@c RETURN_OK for successfull send
	 *         -Everything else triggers sending failed event with
	 *          returnvalue as parameter 1
	 */
	virtual ReturnValue_t sendMessage(Cookie *cookie, const uint8_t *data,
			uint32_t len) = 0;

	virtual ReturnValue_t getSendSuccess(Cookie *cookie) = 0;

	/**
	 * Called by DHB in the SEND_WRITE doSendRead().
	 * Instructs the Communication Interface to prepare
	 * @param cookie
	 * @return
	 */
	virtual ReturnValue_t requestReceiveMessage(Cookie *cookie) = 0;

	/**
	 * Called by DHB in the GET_WRITE doGetRead().
	 * This function is used to receive data from the physical device
	 * by implementing and calling related drivers or wrapper functions.
	 * @param cookie
	 * @param data
	 * @param len
	 * @return @c RETURN_OK for successfull receive
	 *         Everything else triggers receiving failed with returnvalue as parameter 1
	 */
	virtual ReturnValue_t readReceivedMessage(Cookie *cookie, uint8_t **buffer,
			uint32_t *size) = 0;

	virtual ReturnValue_t setAddress(Cookie *cookie, address_t address) = 0;

	virtual address_t getAddress(Cookie *cookie) = 0;

	/**
	 * Can be used by DeviceHandlerBase getParameter() call to set DeviceComIF parameters
	 * @param cookie
	 * @param parameter
	 * @return
	 */
	virtual ReturnValue_t setParameter(Cookie *cookie, uint32_t parameter) = 0;

	/**
	 * Can be used by DeviceHandlerBase getParameter() call to set DeviceComIF parameters
	 * @param cookie
	 * @param parameter
	 * @return
	 */
	virtual uint32_t getParameter(Cookie *cookie) = 0;
};

#endif /* DEVICECOMMUNICATIONIF_H_ */
