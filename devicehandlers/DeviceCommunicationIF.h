#ifndef DEVICECOMMUNICATIONIF_H_
#define DEVICECOMMUNICATIONIF_H_

#include <framework/devicehandlers/Cookie.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>

/**
 * Documentation: Dissertation Baetz p.138
 *
 * This is an interface to decouple device communication from
 * the device handler to allow reuse of these components.
 * It works with the assumption that received data
 * is polled by a component. There are four generic steps of device communication:
 *
 *   1. Send data to a device
 *   2. Get acknowledgement for sending
 *   3. Request reading data from a device
 *   4. Read received data
 *
 *  To identify different connection over a single interface can return so-called cookies to components.
 *
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

	virtual ~DeviceCommunicationIF() {

	}

	virtual ReturnValue_t open(Cookie **cookie, uint32_t address,
			uint32_t maxReplyLen) = 0;

	/**
	 * Use an existing cookie to open a connection to a new DeviceCommunication.
	 * The previous connection must not be closed.
	 * If the returnvalue is not RETURN_OK, the cookie is unchanged and
	 * can be used with the previous connection.
	 *
	 * @param cookie
	 * @param address
	 * @param maxReplyLen
	 * @return
	 */
	virtual ReturnValue_t reOpen(Cookie *cookie, uint32_t address,
			uint32_t maxReplyLen) = 0;

	virtual void close(Cookie *cookie) = 0;

	/**
	 * Called by DHB in the SEND_WRITE doSendWrite().
	 * This function is used to send data to the physical device
	 * by implementing and calling related drivers or wrapper functions.
	 * @param cookie
	 * @param data
	 * @param len
	 * @return
	 */
	virtual ReturnValue_t sendMessage(Cookie *cookie,const uint8_t *data,
			uint32_t len) = 0;

	virtual ReturnValue_t getSendSuccess(Cookie *cookie) = 0;

	virtual ReturnValue_t requestReceiveMessage(Cookie *cookie) = 0;

	/**
	 * Called by DHB in the GET_WIRTE doGetRead().
	 * This function is used to receive data from the physical device
	 * by implementing and calling related drivers or wrapper functions.
	 * @param cookie
	 * @param data
	 * @param len
	 * @return
	 */
	virtual ReturnValue_t readReceivedMessage(Cookie *cookie, uint8_t **buffer,
			uint32_t *size) = 0;

	virtual ReturnValue_t setAddress(Cookie *cookie, uint32_t address) = 0;

	virtual uint32_t getAddress(Cookie *cookie) = 0;

	virtual ReturnValue_t setParameter(Cookie *cookie, uint32_t parameter) = 0;

	virtual uint32_t getParameter(Cookie *cookie) = 0;

};

#endif /* DEVICECOMMUNICATIONIF_H_ */
