#ifndef MISSION_RMAP_RMAPDEVICECOMMUNICATIONINTERFACE_H_
#define MISSION_RMAP_RMAPDEVICECOMMUNICATIONINTERFACE_H_

#include "../devicehandlers/DeviceCommunicationIF.h"

/**
 * @brief	This class is a implementation of a DeviceCommunicationIF for RMAP calls. It expects RMAPCookies or a derived class of RMAPCookies
 *
 * @details	The open, close and reOpen calls are mission specific
 * 			The open call might return any child of RMAPCookies
 *
 * \ingroup rmap
 */
class RmapDeviceCommunicationIF: public DeviceCommunicationIF {

public:
	virtual ~RmapDeviceCommunicationIF();


	/**
	 * This method is mission specific as the open call will return a mission specific cookie
	 *
	 * @param cookie A cookie, can be mission specific subclass of RMAP Cookie
	 * @param address The address of the RMAP Cookie
	 * @param maxReplyLen Maximum length of expected reply
	 * @return
	 */
	virtual ReturnValue_t open(CookieIF **cookie, uint32_t address,
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
	virtual ReturnValue_t reOpen(CookieIF *cookie, uint32_t address,
			uint32_t maxReplyLen) = 0;


	/**
	 * Closing call of connection and memory free of cookie. Mission dependent call
	 * @param cookie
	 */
	virtual void close(CookieIF *cookie) = 0;

	//SHOULDDO can data be const?
	/**
	 *
	 *
	 * @param cookie Expects an RMAPCookie or derived from RMAPCookie Class
	 * @param data Data to be send
	 * @param len Length of the data to be send
	 * @return - Return codes of RMAP::sendWriteCommand()
	 */
	virtual ReturnValue_t sendMessage(CookieIF *cookie, uint8_t *data,
			uint32_t len);

	virtual ReturnValue_t getSendSuccess(CookieIF *cookie);

	virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie);

	virtual ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer,
			uint32_t *size);

	virtual ReturnValue_t setAddress(CookieIF *cookie, uint32_t address);

	virtual uint32_t getAddress(CookieIF *cookie);

	virtual ReturnValue_t setParameter(CookieIF *cookie, uint32_t parameter);

	virtual uint32_t getParameter(CookieIF *cookie);
};

#endif /* MISSION_RMAP_RMAPDEVICECOMMUNICATIONINTERFACE_H_ */
